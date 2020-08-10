#include "formal_grammar.h"

#include "linked_list.h"
#include "log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

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
