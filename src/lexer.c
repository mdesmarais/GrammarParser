#include "lexer.h"

#include "formal_grammar.h"
#include "linked_list.h"
#include "log.h"
#include "string_utils.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

static const char DIGITS[] = {"0123456789" };
static const char LETTERS[] = {"abcdefghijklmnopqrstuvwxyz" };

static int createRange(lex_Range *range, char x, char y, const char *seq) {
    assert(range);
    assert(seq);

    const char *start = strchr(seq, x);
    const char *end = strchr(seq, y);

    if (!start || !end) {
        return LEXER_INVALID_CHAR_RANGE;
    }

    if (start > end) {
        return LEXER_INVALID_RANGE;
    }

    range->start = start;
    range->end = end + 1;

    return LEXER_OK;
}

int lex_createDigitRange(lex_Range *range, char n1, char n2) {
    assert(range);

    return createRange(range, n1, n2, DIGITS);
}

int lex_createLetterRange(lex_Range *range, char c1, char c2, bool uppercase) {
    assert(range);

    range->uppercaseLetter = uppercase;

    if (uppercase) {
        c1 = tolower(c1);
        c2 = tolower(c2);
    }

    return createRange(range, c1, c2, LETTERS);
}

/*bool lex_matchInRange(lex_Range *range, char c, bool isLetter) {
    assert(range);

    for (const char *curr = range->start; curr != range->end; ++curr) {
        if ((isLetter && range->uppercaseLetter && toupper(*curr) == c) || *curr == c) {
            return true;
        }
    }

    return false;
} */

int lex_extractRange(lex_Range *range, const char *input) {
    assert(range);
    assert(input);

    char c1, c2;

    int extractedElements = sscanf(input, "%c-%c", &c1, &c2);

    if (extractedElements != 2) {
        return LEXER_INVALID_RANGE_PATTERN;
    }

    if (isalpha(c1) && isalpha(c2)) {
        return lex_createLetterRange(range, c1, c2, isupper(c1) && isupper(c2));
    }
    else if (isdigit(c1) && isdigit(c2)) {
        return lex_createDigitRange(range, c1, c2);
    }
    else {
        return LEXER_INVALID_RANGE_PATTERN;
    }
}

int lex_extractRanges(lex_Range **pRanges, const char *input, size_t length) {
    assert(pRanges);
    assert(input);

    char *buffer = calloc(length, 1);

    if (!buffer) {
        return -1;
    }

    size_t lengthWithoutSpaces = str_removeWhitespaces(buffer, input, length);

    if (lengthWithoutSpaces == 0) {
        free(buffer);
        return 0;
    }

    if (lengthWithoutSpaces % 3 != 0) {
        free(buffer);
        return -1;
    }

    size_t rangesNumber = lengthWithoutSpaces / 3;
    lex_Range *ranges = calloc(rangesNumber, sizeof(*ranges));

    if (!ranges) {
        free(buffer);
        return -1;
    }

    const char *pos = input;
    size_t i;

    for (i = 0;i < rangesNumber;++i) {
        if (lex_extractRange(ranges + i, pos) != LEXER_OK) {
            free(buffer);
            free(ranges);
            return -1;
        }
        pos += 3;
    }

    free(buffer);
    *pRanges = ranges;

    return i;
}

int lex_extractGrammarItems(const char *source, size_t length, ll_LinkedList *itemList) {
    assert(source);
    assert(itemList);

    // This buffer will hold
    char *buffer = malloc(length);

    if (!buffer) {
        return -1;
    }

    size_t sourcePos = 0;
    int extractedItems = 0;

    while (sourcePos < length) {
        *buffer = '\0';
        const char *currentPosPtr = source + sourcePos;
        //log_debug("loop, pos=%ld str=%s", pos, target);

        // Looking for a string block, we don't want to remove any spaces from it
        char *startBlockPos = memchr(currentPosPtr, '`', length - sourcePos);

        if (startBlockPos) {
            // We have the position of the block's start, then we need to find its end
            char *endBlockPos = memchr(startBlockPos + 1, '`', length - (startBlockPos - currentPosPtr + 1));
            if (!endBlockPos) {
                log_error("Missing end of string block");
                return -1;
            }

            // Ensuring the current character is not the start of a block
            if (currentPosPtr != startBlockPos) {
                size_t blockLength = str_removeMultipleSpaces(buffer, currentPosPtr, startBlockPos - currentPosPtr);

                int result = str_splitItems(buffer, blockLength, itemList, ' ');

                if (result == -1) {
                    log_error("Items extraction failed (1)");
                    return -1;
                }

                extractedItems += result;

                size_t stringBlockLength = endBlockPos - startBlockPos + 1;
                char *stringBlock = malloc(stringBlockLength + 1);

                if (!stringBlock) {
                    log_error("string block allocation error");
                    return -1;
                }

                stringBlock[stringBlockLength] = '\0';
                memcpy(stringBlock, startBlockPos, stringBlockLength);

                ll_pushBack(itemList, stringBlock);
                extractedItems += 1;
            }

            size_t stringBlockLength = endBlockPos - currentPosPtr + 1;
            sourcePos += stringBlockLength;
        }
        else {
            ssize_t blockSize = str_removeMultipleSpaces(buffer, currentPosPtr, strlen(currentPosPtr));
            int result = str_splitItems(buffer, blockSize, itemList, ' ');

            if (result == -1) {
                log_error("Items extraction failed (2)");
                return -1;
            }

            extractedItems += result;
            break;
        }
    }

    free(buffer);

    const char *delimiters = "+|?;=";
    ll_Iterator it = ll_createIterator(itemList);
    int pwet = lex_splitDelimiters(&it, delimiters, strlen(delimiters));

    return extractedItems + pwet;
}

ssize_t lex_readGrammar(FILE *stream, char **pBuffer) {
    assert(stream);
    assert(pBuffer);

    char *fileContent = NULL;
    size_t capacity = 0;
    size_t pos = 0;

    char buffer[512] = { '\0' };

    while (fgets(buffer, 512, stream)) {
        size_t charsRead = strlen(buffer);

        if (charsRead == 0) {
            break;
        }

        if (pos + charsRead > capacity) {
            size_t newCapacity = capacity;

            while (newCapacity < pos + charsRead) {
                newCapacity += 512;
            }
            char *newFileContentBuffer = realloc(fileContent, newCapacity);

            if (!newFileContentBuffer) {
                free(fileContent);
                return -1;
            }

            capacity = newCapacity;
            fileContent = newFileContentBuffer;
        }

        memcpy(fileContent + pos, buffer, charsRead);
        pos += charsRead;

        *buffer = '\0';
    }

    *(fileContent + pos) = '\0';
    *pBuffer = fileContent;

    return pos;
}

int lex_parseGrammarItems(fg_Grammar *g, ll_LinkedList *itemList) {
    assert(g);
    assert(itemList);

    ll_Iterator it = ll_createIterator(itemList);

    while (ll_iteratorHasNext(&it)) {
        char *item = ll_iteratorNext(&it);

        if (!isalpha(*item)) {
            // error
            log_error("unknown item %s", item);
            return LEXER_UNKNOWN_ITEM;
        }

        if (isupper(*item)) {
            // it should be a token
            fg_Token *token = malloc(sizeof(*token));
            memset(token, 0, sizeof(*token));

            int errCode = fg_extractToken(token, &it, item);

            if (errCode != FG_OK) {
                fg_freeToken(token);
                free(token);
                log_error("Token extraction error");
                return errCode;
            }

            ht_insertElement(&g->tokens, token->name, token);
        }
        else {
            // it should be a rule
            fg_Rule *rule = malloc(sizeof(*rule));
            fg_createRule(rule);

            int errCode = fg_extractRule(rule, &it, item);

            if (errCode != FG_OK) {
                fg_freeRule(rule);
                free(rule);
                log_error("Rule extraction error");
                return errCode;
            }

            ht_insertElement(&g->rules, rule->name, rule);
        }
    }

    return LEXER_OK;
}

struct ResolverArg {
    fg_Grammar *g;
    fg_Rule *rule;
    int errCode;
};

static void resolveProductionRulesSymbols(void *data, void *args) {
    ll_LinkedList *pr = data;
    struct ResolverArg *resolverArg = args;

    ll_Iterator it;
    ll_initIterator(&it, pr);

    while (ll_iteratorHasNext(&it)) {
        fg_PRItem *prItem = ll_iteratorNext(&it);

        if (prItem->type == FG_RULE_ITEM) {
            fg_Rule *refRule = ht_getValue(&resolverArg->g->rules, prItem->symbol);

            if (!refRule) {
                resolverArg->errCode = FG_UNKNOWN_RULE;
                log_error("Unknown rule %s for %s", prItem->symbol, resolverArg->rule->name);
                break;
            }
        }
        else if (prItem->type == FG_TOKEN_ITEM) {
            fg_Token *refToken = ht_getValue(&resolverArg->g->tokens, prItem->symbol);

            if (!refToken) {
                resolverArg->errCode = FG_UNKNOWN_TOKEN;
                log_error("Unknown token %s for %s", prItem->symbol, resolverArg->rule->name);
                break;
            }
        }
    }
}

int lex_resolveSymbols(fg_Grammar *g) {
    ht_Iterator tokensIt;
    ht_createIterator(&tokensIt, &g->tokens);

    ht_Iterator rulesIt;
    ht_createIterator(&rulesIt, &g->rules);

    while (ht_iteratorHasNext(&tokensIt)) {
        ht_KVPair *pair = ht_iteratorNext(&tokensIt);
        fg_Token *token = pair->value;

        if (token->type == FG_REF_TOKEN) {
            fg_Token *refToken = ht_getValue(&g->tokens, token->value.refToken);

            if (!refToken) {
                log_error("Unknown token %s for %s", token->value.refToken, token->name);
                return FG_UNKNOWN_TOKEN;
            }

            // @TODO
        }
    }

    struct ResolverArg resolverArg = { .g = g, .errCode = FG_OK };

    while (ht_iteratorHasNext(&rulesIt)) {
        ht_KVPair *pair = ht_iteratorNext(&rulesIt);
        fg_Rule *rule = pair->value;

        resolverArg.rule = rule;

        ll_forEachItem(&rule->productionRuleList, resolveProductionRulesSymbols, &resolverArg);

        if (resolverArg.errCode != FG_OK) {
            return resolverArg.errCode;
        }
    }

    return LEXER_OK;
}

int lex_splitDelimiters(ll_Iterator *it, const char *delimiters, size_t delimitersCount) {
    assert(it);
    assert(delimiters);

    int extractedItems = 0;

    while (ll_iteratorHasNext(it)) {
        char *item = ll_iteratorNext(it);

        if (*item == '`') {
            // If the item is a string block then it does not
            // require to be splitter
            continue;
        }

        char *ptr = str_findDelimiter(item, strlen(item), delimiters, delimitersCount);

        if (ptr && strlen(item) > 1) {
            // A 1-char length item does not require to be splitted

            if (ptr == item) {
                // If the item starts with a delimiter then we should
                // keep it and put following characters into a new item.
                ++ptr;
            }

            // Length of the new item
            size_t length = item + strlen(item) - ptr;
            char *newItem = malloc(length + 1);
            memcpy(newItem, ptr, length);
            newItem[length] = '\0';

            // Inserts the new item into the list right after the current item
            ll_iteratorInsert(it, newItem);

            // Adds a null character at the position of the delimiter
            // We only keep the part that is before the delimiter
            // or the character itself in the case of ptr==item.
            *ptr = '\0';

            ++extractedItems;
        }
    }

    return extractedItems;
}
