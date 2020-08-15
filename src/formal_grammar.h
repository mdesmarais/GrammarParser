#ifndef LEXER_FORMAL_GRAMMAR_H
#define LEXER_FORMAL_GRAMMAR_H

#include "hash_table.h"
#include "lexer.h"
#include "linked_list.h"

#include <string.h>

struct fg_ProductionRule;
struct fg_PRItem;

typedef enum fg_TokenType {
    FG_RANGE_TOKEN,
    FG_STRING_TOKEN
} fg_TokenType;

typedef struct fg_Token {
    fg_TokenType type;
    char *name;
    char *string;
    lex_Range *ranges;
    int rangesNumber;
} fg_Token;

typedef struct fg_Rule {
    char *name;
    ll_LinkedList productionRuleList;
} fg_Rule;

typedef enum fg_PrItemType {
    FG_RULE_ITEM,
    FG_TOKEN_ITEM
} fg_PrItemType;

typedef struct fg_PRItem {
    fg_PrItemType type;
    fg_Rule *rule;
    fg_Token *token;
} fg_PRItem;

typedef struct fg_Grammar {
    ht_Table tokens;
    ht_Table rules;
} fg_Grammar;

typedef enum fg_ErrorCode {
    FG_OK,
    FG_TOKEN_INVALID,
    FG_TOKEN_MISSING_END,
    FG_TOKEN_MISSING_VALUE,
    FG_TOKEN_INVALID_VALUE,
    FG_TOKEN_UNKNOWN_VALUE_TYPE,

    FG_RULE_EMPTY,
    FG_RULE_INVALID,
    FG_RULE_MISSING_END,
    FG_RULE_MISSING_VALUE,

    FG_UNKNOWN_TOKEN,
    FG_UNKNOWN_RULE,

    FG_PR_EMPTY,

    FG_PRITEM_UNKNOWN_TYPE
} fg_ErrorCode;

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
 * @param tokenName name of the token
 * @return FG_OK if not error occured
 */
int fg_extractToken(fg_Token *token, ll_Iterator *it, const char *tokenName);

/**
 * Frees allocated memory in a token.
 *
 * Pointers name and string will be set to NULL
 *
 * @param token pointer to a token structure
 */
void fg_freeToken(fg_Token *token);

int fg_extractRule(fg_Rule *rule, ll_Iterator *it, fg_Grammar *g, const char *ruleName);

void fg_freeRule(fg_Rule *rule);

int fg_extractProductionRule(ll_LinkedList *prItemList, ll_Iterator *it, fg_Grammar *grammar, char *currentItem, char **pLastItem);

void fg_freeProductionRule(ll_LinkedList *pr);

int fg_extractPrItem(fg_PRItem *prItem, const char *item, ll_Iterator *tokenIt, ll_Iterator *ruleIt);

/**
 * Find a token by its name.
 *
 * @param name name of the token, must be null terminated
 * @param tokenIt iterator on a list of tokens
 * @return a pointer to the matched token, or null if no token was found
 */
fg_Token *fg_getTokenByName(const char *name, ll_Iterator *tokenIt);

/**
 * Finds a rule by its name.
 *
 * @param name name of the rule, must be null terminated
 * @param ruleIt iterator on a list of rules
 * @return a pointer to the matched rule, or null if no rule was found
 */
fg_Rule *fg_getRuleByName(const char *name, ll_Iterator *ruleIt);

#endif //LEXER_FORMAL_GRAMMAR_H
