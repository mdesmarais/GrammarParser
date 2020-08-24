#include "formal_grammar.h"

#include "log.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static void tokenDestructor(void *key, void *value) {
    // As the key (string) is the same pointer as
    // the one in the token (name field), we don't need to free it.
    // fg_freeToken will be in charge of it.
    key;
    fg_freeToken((fg_Token*) value);
    free(value);
}

static void ruleDestructor(void *key, void *value) {
    // As the key (string) is the same pointer as
    // the one in the rule (name field), we don't need to free it.
    // fg_freeRule will be in charge of it.
    key;
    fg_freeRule((fg_Rule*) value);
    free(value);
}

void fg_createGrammar(fg_Grammar *g) {
    g->entry = NULL;
    ht_createTable(&g->tokens, 10, ht_hashString, (ht_KeyComparator*) strcmp, tokenDestructor);
    ht_createTable(&g->rules, 10, ht_hashString, (ht_KeyComparator*) strcmp, ruleDestructor);
}

void fg_freeGrammar(fg_Grammar *g) {
    if (g) {
        ht_freeTable(&g->rules);
        ht_freeTable(&g->tokens);
        g->entry = NULL;
    }
}

#define expectCharFromIt(it, expected, ret) do { \
    if (!ll_iteratorHasNext((it)) || *((prs_StringItem*) ll_iteratorNext((it)))->item != (expected)) { \
        return (ret);                                   \
    }                                                   \
} while(0);

prs_ErrCode fg_extractToken(fg_Token *token, ll_Iterator *it, const char *tokenName) {
    assert(token);
    assert(it);
    assert(tokenName);

    token->name = calloc(strlen(tokenName) + 1, 1);
    strcpy(token->name, tokenName);

    expectCharFromIt(it, '=', FG_TOKEN_INVALID);

    if (!ll_iteratorHasNext(it)) {
        fg_freeToken(token);
        return FG_TOKEN_MISSING_VALUE;
    }

    prs_StringItem *tokenValueItem = ll_iteratorNext((it));
    char *tokenValue = tokenValueItem->item;

    if (*tokenValue == ';') {
        fg_freeToken(token);
        return FG_TOKEN_MISSING_VALUE;
    }

    if (*tokenValue == '`') {
        size_t length = strlen(tokenValue);
        char *string = calloc(length - 1, 1); // +1 for the null character, -2 for the 2 "`" chars before and after the string
        strncpy(string, tokenValue + 1, length - 2);

        token->type = FG_STRING_TOKEN;
        token->value.string = string;
    }
    else if (*tokenValue == '[') {
        token->type = FG_RANGE_TOKEN;

        struct fg_RangesToken *rangesToken = &token->value.rangesToken;

        // lex_extractRanges expects a string without square brackets : [...]
        int extractedRanges = prs_extractRanges(&rangesToken->ranges, tokenValue + 1, strlen(tokenValue) - 2);

        if (extractedRanges <= 0) {
            return -1;
        }

        rangesToken->rangesNumber = extractedRanges;
    }
    else if (isalpha(*tokenValue) && isupper(*tokenValue)) {
        if (strcmp(tokenValue, tokenName) == 0) {
            return FG_TOKEN_SELF_REF;
        }

        char *symbol = calloc(strlen(tokenValue) + 1, 1);
        strcpy(symbol, tokenValue);

        token->type = FG_REF_TOKEN;
        token->value.refToken.symbol = symbol;
    }
    else {
        fg_freeToken(token);
        return FG_TOKEN_UNKNOWN_VALUE_TYPE;
    }

    if (!ll_iteratorHasNext(it)) {
        fg_freeToken(token);
        return FG_TOKEN_MISSING_END;
    }

    char c = *((prs_StringItem *) ll_iteratorNext(it))->item;
    prs_RangeQuantifier quantifier = -1;

    switch (c) {
        case '+':
            quantifier = PRS_PLUS_QUANTIFIER;
            expectCharFromIt(it, ';', FG_TOKEN_MISSING_END);
            break;
        case '?':
            quantifier = PRS_QMARK_QUANTIFIER;
            expectCharFromIt(it, ';', FG_TOKEN_MISSING_END);
            break;
        case ';':
            break;
        default:
            fg_freeToken(token);
            return FG_RULE_MISSING_END;
    }

    if (quantifier >= 0) {
        token->quantifier = quantifier;
    }

    return PRS_OK;
}

void fg_freeToken(fg_Token *token) {
    if (token) {
        free(token->name);

        switch (token->type) {
            case FG_RANGE_TOKEN:
                free(token->value.rangesToken.ranges);
                break;
            case FG_REF_TOKEN:
                free(token->value.refToken.symbol);
                break;
            case FG_STRING_TOKEN:
                free(token->value.string);
                break;
            default:
                break;
        }

        memset(token, 0, sizeof(*token));
    }
}

static void prItemDestructor(void *data, void *args) {
    fg_PRItem *prItem = data;
    fg_freePrItem(prItem);
    free(prItem);
}

prs_ErrCode fg_extractRule(fg_Rule *rule, ll_Iterator *it, const char *ruleName) {
    assert(rule);
    assert(it);
    assert(ruleName);

    rule->name = calloc(strlen(ruleName) + 1, 1);
    strcpy(rule->name, ruleName);

    expectCharFromIt(it, '=', FG_RULE_INVALID);

    bool hasEnd = false;
    int extractedProductionRules = 0;

    while (ll_iteratorHasNext(it)) {
        prs_StringItem *currentStringItem = ll_iteratorNext(it);
        char *currentItem = currentStringItem->item;

        if (*currentItem == ';') {
            hasEnd = true;
            break;
        }

        ll_LinkedList *productionRule = malloc(sizeof(*productionRule));
        ll_createLinkedList(productionRule, prItemDestructor);

        char *lastItem = NULL;
        int errCode = fg_extractProductionRule(productionRule, it, currentItem, &lastItem);

        if (errCode != PRS_OK) {
            free(productionRule);
            fg_freeRule(rule);
            return errCode;
        }

        ll_pushBack(&rule->productionRuleList, productionRule);
        ++extractedProductionRules;

        if (lastItem && *lastItem == ';') {
            hasEnd = true;
            break;
        }
    }

    if (extractedProductionRules == 0) {
        fg_freeRule(rule);
        return FG_RULE_EMPTY;
    }

    if (!hasEnd) {
        fg_freeRule(rule);
        return FG_RULE_MISSING_END;
    }

    return PRS_OK;
}

static void productionRuleDestructor(ll_LinkedList *pr) {
    ll_freeLinkedList(pr, NULL);
    free(pr);
}

void fg_createRule(fg_Rule *rule) {
    rule->name = NULL;
    ll_createLinkedList(&rule->productionRuleList, (ll_DataDestructor*) productionRuleDestructor);
}

void fg_freeRule(fg_Rule *rule) {
    if (rule) {
        free(rule->name);
        ll_freeLinkedList(&rule->productionRuleList, NULL);

        rule->name = NULL;
    }
}

prs_ErrCode fg_extractProductionRule(ll_LinkedList *prItemList, ll_Iterator *it, char *currentItem, char **pLastItem) {
    assert(prItemList);
    assert(it);
    assert(currentItem);
    assert(pLastItem);

    char *item = currentItem;
    char *lastItem = NULL;

    int extractedItems = 0;

    do {
        if (*item == '|' || *item == ';') {
            lastItem = item;
            break;
        }

        fg_PRItem *prItem = malloc(sizeof(*prItem));
        memset(prItem, 0, sizeof(*prItem));

        int errCode = fg_extractPrItem(prItem, item);

        if (errCode != PRS_OK) {
            ll_freeLinkedList(prItemList, NULL);
            free(prItem);
            return errCode;
        }

        ll_pushBack(prItemList, prItem);
        ++extractedItems;
    } while (ll_iteratorHasNext(it) && (item = ((prs_StringItem*) ll_iteratorNext(it))->item));

    if (extractedItems == 0) {
        return FG_PR_EMPTY;
    }

    *pLastItem = lastItem;

    return PRS_OK;
}

prs_ErrCode fg_extractPrItem(fg_PRItem *prItem, const char *item) {
    assert(prItem);
    assert(item);

    if (!isalpha(*item)) {
        return FG_PRITEM_UNKNOWN_TYPE;
    }

    char *symbol = calloc(strlen(item) + 1, 1);
    strcpy(symbol, item);
    prItem->symbol = symbol;

    prItem->type = (islower(*item)) ? FG_RULE_ITEM : FG_TOKEN_ITEM;

    return PRS_OK;
}

void fg_freePrItem(fg_PRItem *prItem) {
    if (prItem) {
        free(prItem->symbol);
    }
}
