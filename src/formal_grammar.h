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
    const char *name;
    const char *string;
    lex_Range range;
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

int fg_extractToken(fg_Token *token, struct ll_Iterator *itemIterator, char *currentItem);

#endif //LEXER_FORMAL_GRAMMAR_H
