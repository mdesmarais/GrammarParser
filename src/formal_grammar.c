#include "formal_grammar.h"

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
} while(0)

prs_ErrCode fg_extractToken(fg_Token *token, ll_Iterator *it, prs_StringItem *tokenNameItem) {
    assert(token);
    assert(it);
    assert(tokenNameItem);

    // We don't need the prefix (%)
    const char *tokenName = tokenNameItem->item + 1;

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

        // lex_extractRanges expects a string without square brackets : [...]
        prs_ErrCode errCode = prs_extractRanges(&token->value.rangeArray, tokenValue + 1, strlen(tokenValue) - 2);

        if (errCode != PRS_OK) {
            fg_freeToken(token);
            return errCode;
        }
    }
    else if (isalpha(*tokenValue) && isupper(*tokenValue)) {
        if (strcmp(tokenValue, tokenName) == 0) {
            fg_freeToken(token);
            return FG_TOKEN_SELF_REF;
        }

        token->type = FG_REF_TOKEN;
        token->value.refToken.symbol = tokenValueItem;
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
        case '*':
            quantifier = PRS_STAR_QUANTIFIER;
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
                prs_freeRangeArray(&token->value.rangeArray);
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

static void prItemDestructor(fg_PRItem *prItem) {
    fg_freePRItem(prItem);
    free(prItem);
}

prs_ErrCode fg_extractRule(fg_Rule *rule, ll_Iterator *it, prs_StringItem *ruleNameItem) {
    assert(rule);
    assert(it);
    assert(ruleNameItem);

    // We don't need the prefix (%)
    const char *ruleName = ruleNameItem->item + 1;

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
        ll_createLinkedList(productionRule, (ll_DataDestructor*) prItemDestructor);

        prs_StringItem *lastStringItem = NULL;
        int errCode = fg_extractProductionRule(productionRule, it, currentStringItem, &lastStringItem);

        if (errCode != PRS_OK) {
            free(productionRule);
            fg_freeRule(rule);
            return errCode;
        }

        ll_pushBack(&rule->productionRuleList, productionRule);
        ++extractedProductionRules;

        if (lastStringItem && *lastStringItem->item == ';') {
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

prs_ErrCode fg_extractProductionRule(ll_LinkedList *prItemList, ll_Iterator *it, prs_StringItem *currentStringItem, prs_StringItem **pLastStringItem) {
    assert(prItemList);
    assert(it);
    assert(currentStringItem);
    assert(pLastStringItem);

    prs_StringItem *lastStringItem = NULL;

    int extractedItems = 0;

    do {
        const char *item = currentStringItem->item;
        if (*item == '|' || *item == ';') {
            lastStringItem = currentStringItem;
            break;
        }

        fg_PRItem *prItem = malloc(sizeof(*prItem));
        memset(prItem, 0, sizeof(*prItem));

        prs_ErrCode errCode = fg_extractPRItem(prItem, currentStringItem);

        if (errCode != PRS_OK) {
            prs_setErrorState(currentStringItem);
            fg_freePRItem(prItem);
            free(prItem);
            ll_freeLinkedList(prItemList, NULL);
            return errCode;
        }

        ll_pushBack(prItemList, prItem);
        ++extractedItems;
    } while (ll_iteratorHasNext(it) && (currentStringItem = ll_iteratorNext(it)));

    if (extractedItems == 0) {
        return FG_PR_EMPTY;
    }

    *pLastStringItem = lastStringItem;

    return PRS_OK;
}

prs_ErrCode fg_extractPRItem(fg_PRItem *prItem, prs_StringItem *stringItem) {
    assert(prItem);
    assert(stringItem);

    const char *item = stringItem->item;

    if (*item == '`') {
        size_t blockLength = strlen(item);

        if (blockLength < 2 || !strchr(item + 1, '`')) {
            return FG_STRING_BLOCK_MISSING_END;
        }

        if (blockLength == 2) {
            return FG_STRING_BLOCK_EMPTY;
        }

        prItem->type = FG_STRING_ITEM;
        prItem->value.string = calloc(blockLength + 1, 1);
        strncpy(prItem->value.string, item + 1, blockLength - 2);
    }
    else if (!isalpha(*item)) {
        return FG_PRITEM_UNKNOWN_TYPE;
    }
    else {
        prItem->type = (islower(*item)) ? FG_RULE_ITEM : FG_TOKEN_ITEM;
        prItem->symbol = stringItem;
    }

    return PRS_OK;
}

void fg_freePRItem(fg_PRItem *prItem) {
    if (prItem) {
        switch (prItem->type) {
            case FG_STRING_ITEM:
                free(prItem->value.string);
                break;
            default:
                prItem->symbol = NULL;
                break;
        }
    }
}
