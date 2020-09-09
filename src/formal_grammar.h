#ifndef FORMAL_GRAMMAR_H
#define FORMAL_GRAMMAR_H

#include "collections/hash_table.h"
#include "collections/linked_list.h"
#include "parser.h"
#include "parser_errors.h"
#include "range.h"

#include <string.h>

typedef enum fg_TokenType {
    FG_RANGE_TOKEN,
    FG_REF_TOKEN,
    FG_STRING_TOKEN
} fg_TokenType;

struct fg_Token;

struct fg_RefToken {
    prs_StringItem *symbol;
    struct fg_Token *token;
};

union fg_TokenValue {
    prs_RangeArray rangeArray;
    struct fg_RefToken refToken;
    char *string;
};

typedef struct fg_Token {
    fg_TokenType type;
    char *name;
    prs_RangeQuantifier quantifier;
    union fg_TokenValue value;
} fg_Token;

typedef struct fg_Rule {
    char *name;
    ll_LinkedList productionRuleList;
} fg_Rule;

typedef enum fg_PrItemType {
    FG_RULE_ITEM,
    FG_STRING_ITEM,
    FG_TOKEN_ITEM
} fg_PrItemType;

union fg_PRItemValue {
    fg_Rule *rule;
    char *string;
    fg_Token *token;
};

typedef struct fg_PRItem {
    fg_PrItemType type;
    prs_StringItem *symbol;
    union fg_PRItemValue value;
} fg_PRItem;

typedef struct fg_Grammar {
    ht_Table tokens;
    ht_Table rules;
    fg_Rule *entry;
} fg_Grammar;

void fg_createGrammar(fg_Grammar *g);
void fg_freeGrammar(fg_Grammar *g);

/**
 * Extracts a token from a list of items.
 *
 * A token is made by a name (starting with an uppercase letter),
 * an equal sign, a value (string block or range) and a semicolon.
 *
 * @param token pointer to a structure that will receive values
 * @param it iterator to a list that contains items
 * @param tokenNameItem pointer to the string item
 * @return FG_OK if not error occurs
 */
prs_ErrCode fg_extractToken(fg_Token *token, ll_Iterator *it, prs_StringItem *tokenNameItem);

/**
 * Frees allocated memory in a token.
 *
 * Pointers name and string will be set to NULL
 *
 * @param token pointer to a token structure
 */
void fg_freeToken(fg_Token *token);

prs_ErrCode fg_extractRule(fg_Rule *rule, ll_Iterator *it, prs_StringItem *ruleNameItem);

void fg_createRule(fg_Rule *rule);
void fg_freeRule(fg_Rule *rule);

prs_ErrCode fg_extractProductionRule(ll_LinkedList *prItemList, ll_Iterator *it, prs_StringItem *currentStringItem, prs_StringItem **pLastStringItem);

prs_ErrCode fg_extractPRItem(fg_PRItem *prItem, prs_StringItem *stringItem);
void fg_freePRItem(fg_PRItem *prItem);

#endif //FORMAL_GRAMMAR_H
