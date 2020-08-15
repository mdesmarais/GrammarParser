#include "formal_grammar.h"

#include "log.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static void tokenDestructor(void *data) {
    // As the name in the pair is the same pointer as
    // the one in the token, we don't need to free it.
    // fg_freeToken will be in charge of it.
    ht_KVPair *pair = data;
    fg_freeToken(pair->value);
    free(pair);
}

static void ruleDestructor(void *data) {
    // As the name in the pair is the same pointer as
    // the one in the rule, we don't need to free it.
    // fg_freeRule will be in charge of it.
    ht_KVPair *pair = data;
    fg_freeRule(pair->value);
    free(pair);
}

void fg_createGrammar(fg_Grammar *g) {
    ht_createTable(&g->tokens, 10, ht_hashString, (ht_KeyComparator*) strcmp, tokenDestructor);
    ht_createTable(&g->rules, 10, ht_hashString, (ht_KeyComparator*) strcmp, ruleDestructor);
}

void fg_freeGrammar(fg_Grammar *g) {
    if (g) {
        ht_freeTable(&g->tokens);
        ht_freeTable(&g->tokens);
    }
}

#define expectCharFromIt(it, expected, ret) do { \
    if (!ll_iteratorHasNext((it)) || *((char*) ll_iteratorNext((it))) != (expected)) { \
        return (ret);                                   \
    }                                                   \
} while(0);

int fg_extractToken(fg_Token *token, ll_Iterator *it, const char *tokenName) {
    assert(token);
    assert(it);
    assert(tokenName);

    expectCharFromIt(it, '=', FG_TOKEN_INVALID);

    if (!ll_iteratorHasNext(it)) {
        return FG_TOKEN_MISSING_VALUE;
    }

    char *tokenValue = ll_iteratorNext((it));

    if (*tokenValue == ';') {
        return FG_TOKEN_MISSING_VALUE;
    }

    if (*tokenValue == '`') {
        size_t length = strlen(tokenValue);
        token->type = FG_STRING_TOKEN;
        token->string = calloc(length - 1, 1); // +1 for the null character, -2 for the 2 "`" chars before and after the string
        strncpy(token->string, tokenValue + 1, length - 2);
    }
    else if (*tokenValue == '[') {
        token->type = FG_RANGE_TOKEN;

        // lex_extractRanges expects a string without square brackets : [...]
        int extractedRanges = lex_extractRanges(&token->ranges, tokenValue + 1, strlen(tokenValue) - 2);

        if (extractedRanges <= 0) {
            return -1;
        }

        token->rangesNumber = extractedRanges;
    }
    else {
        return FG_TOKEN_UNKNOWN_VALUE_TYPE;
    }

    if (!ll_iteratorHasNext(it)) {
        return FG_TOKEN_MISSING_END;
    }

    char c = *((char*) ll_iteratorNext(it));
    lex_RangeQuantifier quantifier = -1;

    switch (c) {
        case '+':
            quantifier = LEX_PLUS_QUANTIFIER;
            break;
        case '?':
            quantifier = LEX_QMARK_QUANTIFIER;
            break;
        case ';':
            break;
        default:
            expectCharFromIt(it, ';', FG_TOKEN_MISSING_END);
    }

    if (quantifier >= 0) {
        for (int i = 0;i < token->rangesNumber;++i) {
            token->ranges[i].quantifier = quantifier;
        }
    }

    token->name = malloc(strlen(tokenName) + 1);
    strcpy(token->name, tokenName);

    return FG_OK;
}

void fg_freeToken(fg_Token *token) {
    if (token) {
        free(token->name);
        free(token->string);
        free(token->ranges);

        token->name = token->string = NULL;
        token->ranges = NULL;

        token->rangesNumber = 0;
    }
}

int fg_extractRule(fg_Rule *rule, ll_Iterator *it, fg_Grammar *g, const char *ruleName) {
    assert(rule);
    assert(it);
    assert(g);
    assert(ruleName);

    rule->name = malloc(strlen(ruleName) + 1);
    strcpy(rule->name, ruleName);

    ht_KVPair *pair = malloc(sizeof(*pair));
    ht_createPair(pair, rule->name, rule);
    ht_insertPair(&g->rules, pair);

    expectCharFromIt(it, '=', FG_RULE_INVALID);

    bool hasEnd = false;
    int extractedProductionRules = 0;

    while (ll_iteratorHasNext(it)) {
        char *currentItem = ll_iteratorNext(it);

        if (*currentItem == ';') {
            hasEnd = true;
            break;
        }

        ll_LinkedList *productionRule = malloc(sizeof(*productionRule));
        memset(productionRule, 0, sizeof(*productionRule));

        char *lastItem = NULL;
        int errCode = fg_extractProductionRule(productionRule, it, g, currentItem, &lastItem);

        if (errCode != FG_OK) {
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
        return FG_RULE_EMPTY;
    }

    if (!hasEnd) {
        fg_freeRule(rule);
        return FG_RULE_MISSING_END;
    }

    return FG_OK;
}

void fg_freeRule(fg_Rule *rule) {
    if (rule) {
        free(rule->name);
        ll_freeLinkedList(&rule->productionRuleList, (ll_DataDestructor*) fg_freeProductionRule);

        rule->name = NULL;
    }
}

int fg_extractProductionRule(ll_LinkedList *prItemList, ll_Iterator *it, fg_Grammar *grammar, char *currentItem, char **pLastItem) {
    assert(prItemList);
    assert(it);
    assert(grammar);
    assert(currentItem);
    assert(pLastItem);

    /*ll_Iterator tokenIt = ll_createIterator(&grammar->tokenList);
    ll_Iterator ruleIt = ll_createIterator(&grammar->ruleList);

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

        int errCode = fg_extractPrItem(prItem, item, &tokenIt, &ruleIt);

        if (errCode != FG_OK) {
            ll_freeLinkedList(prItemList, free);
            free(prItem);
            return errCode;
        }

        ll_pushBack(prItemList, prItem);
        ++extractedItems;
    } while (ll_iteratorHasNext(it) && (item = ll_iteratorNext(it)));

    if (extractedItems == 0) {
        return FG_PR_EMPTY;
    }

    *pLastItem = lastItem;*/

    return FG_OK;
}

void fg_freeProductionRule(ll_LinkedList *pr) {
    if (pr) {
        ll_freeLinkedList(pr, free);
        free(pr);
    }
}

int fg_extractPrItem(fg_PRItem *prItem, const char *item, ll_Iterator *tokenIt, ll_Iterator *ruleIt) {
    assert(prItem);
    assert(item);
    assert(tokenIt);
    assert(ruleIt);

    if (!isalpha(*item)) {
        return FG_PRITEM_UNKNOWN_TYPE;
    }

    if (islower(*item)) {
        // it should be a rule
        fg_Rule *rule = fg_getRuleByName(item, ruleIt);

        if (!rule) {
            return FG_UNKNOWN_RULE;
        }

        prItem->type = FG_RULE_ITEM;
        prItem->rule = rule;
    }
    else {
        // it should be a token
        fg_Token *token = fg_getTokenByName(item, tokenIt);

        if (!token) {
            return FG_UNKNOWN_TOKEN;
        }

        prItem->type = FG_TOKEN_ITEM;
        prItem->token = token;
    }

    return FG_OK;
}

fg_Token *fg_getTokenByName(const char *name, struct ll_Iterator *tokenIt) {
    assert(name);
    assert(tokenIt);

    while (ll_iteratorHasNext(tokenIt)) {
        fg_Token *token = ll_iteratorNext(tokenIt);

        if (strcmp(name, token->name) == 0) {
            return token;
        }
    }

    return NULL;
}

fg_Rule *fg_getRuleByName(const char *name, struct ll_Iterator *ruleIt) {
    assert(name);
    assert(ruleIt);

    while (ll_iteratorHasNext(ruleIt)) {
        fg_Rule *rule = ll_iteratorNext(ruleIt);

        if (strcmp(name, rule->name) == 0) {
            return rule;
        }
    }

    return NULL;
}
