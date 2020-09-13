#ifndef FORMAL_GRAMMAR_H
#define FORMAL_GRAMMAR_H

#include "collections/hash_table.h"
#include "collections/linked_list.h"
#include "parser_errors.h"
#include "range.h"

typedef enum fg_TokenType {
    FG_RANGE_TOKEN,
    FG_REF_TOKEN,
    FG_STRING_TOKEN
} fg_TokenType;

struct fg_Token;
struct prs_StringItem;

struct fg_RefToken {
    struct prs_StringItem *symbol;
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
    struct prs_StringItem *symbol;
    union fg_PRItemValue value;
} fg_PRItem;

typedef struct fg_Grammar {
    ht_Table tokens;
    ht_Table rules;
    fg_Rule *entry;
} fg_Grammar;

/**
 * Creates a new grammar.
 *
 * @param g a pointer to a grammar
 */
void fg_createGrammar(fg_Grammar *g);

/**
 * Frees allocated memory for the given grammar.
 *
 * The given pointer will not be freed.
 *
 * @param g a pointer to a grammar
 */
void fg_freeGrammar(fg_Grammar *g);

/**
 * Extracts a token from a list of items.
 *
 * A token is made by a name (starting with an uppercase letter),
 * an equal sign, a value (string block or range) and a semicolon.
 * A token value can be a reference to another token, a range or a string.
 * It can be suffixed by a quantifier : ?, *, +.
 *
 * If the function can not find the token's type, then FG_TOKEN_UNKNOWN_VALUE_TYPE
 * will be returned.
 *
 * If the token's name is not followed by an equal sign (=) or the declaration
 * is not ended by a semicolon then FG_TOKE_INVALID will be returned.
 *
 * If there is no value after the equal sign, then FG_TOKEN_MISSING_VALUE
 * will be returned.
 *
 * Self-referencing token is not allowed : FG_TOKEN_SELF_REF will be returned.
 *
 * Other error codes can be returned by {@link prs_extractRanges}.
 *
 * @param token pointer to a structure that will receive values
 * @param it iterator to a list that contains items
 * @param tokenNameItem pointer to the string item
 * @return FG_OK if not error occurs
 */
prs_ErrCode fg_extractToken(fg_Token *token, ll_Iterator *it, struct prs_StringItem *tokenNameItem);

/**
 * Frees allocated memory in a token.
 *
 * Pointers name and string will be set to NULL.
 * The given pointer will not be freed.
 *
 * @param token pointer to a token structure
 */
void fg_freeToken(fg_Token *token);

/**
 * Extracts a rule from a list of items.
 *
 * A rule is made by one or more production rules.
 * It must be ended by a semicolon : FG_RULE_MISSING_END will be returned
 * if it is not the case.
 *
 * If the rule's name is not followed by an equal char (=) then
 * FG_RULE_INVALID will be returned.
 *
 * If the rule is empty (no production rules) then FG_RULE_EMPTY will be returned.
 *
 * Other error codes can be returned by {@link fg_extractProductionRule}.
 *
 * @param rule a pointer to a rule structure
 * @param it pointer to an iterator over a list of prs_StringItem
 * @param ruleNameItem pointer to a StringItem that represents the rule's name
 * @return PRS_OK if not error occurs, otherwise a different error code
 */
prs_ErrCode fg_extractRule(fg_Rule *rule, ll_Iterator *it, struct prs_StringItem *ruleNameItem);

/**
 * Creates a new rule with default values.
 *
 * @param rule a pointer to a rule
 */
void fg_createRule(fg_Rule *rule);

/**
 * Frees allocated memory for the given rule.
 *
 * The given pointer will not be freed.
 *
 * @param rule a pointer to a rule
 */
void fg_freeRule(fg_Rule *rule);

/**
 * Extracts a production rule from a list of items.
 *
 * A production rule is made by one or more production rule items.
 * It can be ended by a semicolon (end of the rule) or a pipe (more production rules
 * to come). If the production rule is empty then FG_PR_EMPTY will be returned.
 *
 * Other error codes can be returned by {@link fg_extractPRItem}.
 *
 * @param prItemList pointer to a production rule
 * @param it a pointer to an iterator over a list of prs_StringItem
 * @param currentStringItem the current string item
 * @param pLastStringItem pointer to the last string item that not belongs to the production rule
 * @return PRS_OK if no error occurs, otherwise a different error code
 */
prs_ErrCode fg_extractProductionRule(ll_LinkedList *prItemList, ll_Iterator *it, struct prs_StringItem *currentStringItem, struct prs_StringItem **pLastStringItem);

/**
 * Extracts a production rule item from a prs_StringItem.
 *
 * The production rule item can be either a string block : `...`
 * or a reference to a rule / token. If the type of the item is unknown
 * then FG_PRITEM_UNKNOWN_TYPE will be returned.
 *
 * If it is a string block and the end marker "`" is missing, then
 * FG_STRING_BLOCK_MISSING_END will be returned.
 * If the block's content is empty then FG_STRING_BlOCK_EMPTY will be returned.
 *
 * If it is a reference to a rule / token, then this function will not check if the
 * reference exists. This step will be done by {@link prs_resolveSymbols}.
 *
 * @param prItem a pointer to a production rule item
 * @param stringItem stringItem that should be converted into a fg_PRItem
 * @return PRS_OK if no error occurs, otherwise a different error code
 */
prs_ErrCode fg_extractPRItem(fg_PRItem *prItem, struct prs_StringItem *stringItem);

/**
 * Frees allocated memory for the given production rule item.
 *
 * The given pointer will not be freed.
 *
 * @param prItem a pointer to a production rule item
 */
void fg_freePRItem(fg_PRItem *prItem);

#endif //FORMAL_GRAMMAR_H
