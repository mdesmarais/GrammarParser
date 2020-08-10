#ifndef LEXER_FORMAL_GRAMMAR_H
#define LEXER_FORMAL_GRAMMAR_H

#include "lexer.h"

#include <string.h>

struct fg_ProductionRule;
struct fg_PRItem;

struct ll_Iterator;

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
    size_t nameLength;
    struct fg_ProductionRule *pr;
} fg_Rule;

typedef struct fg_ProductionRule {
    struct fg_PRItem *prItem;
    struct fg_ProductionRule *nextPr;
} fg_ProductionRule;

typedef enum fg_PrItemType {
    FG_RULE_ITEM,
    FG_TOKEN_ITEM
} fg_PrItemType;

typedef struct fg_PRTokenItem {

} fg_PRTokenItem;

typedef struct fg_PRItem {
    fg_PrItemType type;
    fg_Rule *rule;
    fg_PRTokenItem token;
    struct fg_PRItem *nextPrItem;
} fg_PRItem;

typedef struct fg_Grammar {

} fg_Grammar;

typedef enum fg_ErrorCode {
    FG_OK,
    FG_TOKEN_INVALID,
    FG_TOKEN_MISSING_END,
    FG_TOKEN_MISSING_VALUE,
    FG_TOKEN_INVALID_VALUE,
    FG_TOKEN_UNKNOWN_VALUE_TYPE
} fg_ErrorCode;

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
int fg_extractToken(fg_Token *token, struct ll_Iterator *it, const char *tokenName);

/**
 * Frees allocated memory in a token.
 *
 * Pointers name and string will be set to NULL
 *
 * @param token pointer to a token structure
 */
void fg_freeToken(fg_Token *token);

#endif //LEXER_FORMAL_GRAMMAR_H
