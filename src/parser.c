#include "parser.h"

#include "formal_grammar.h"
#include "linked_list.h"
#include "log.h"
#include "set.h"
#include "string_utils.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

static prs_ErrCode createRange(prs_Range *range, uint8_t x, uint8_t y, uint8_t asciiStart, uint8_t asciiEnd) {
    assert(range);

    if (x > y) {
        return PRS_INVALID_RANGE;
    }

    if (x < asciiStart || y > asciiEnd) {
        return PRS_INVALID_CHAR_RANGE;
    }

    range->start = x;
    range->end = y + 1;

    return PRS_OK;
}

prs_ErrCode prs_createDigitRange(prs_Range *range, char n1, char n2) {
    assert(range);

    return createRange(range, n1, n2, PRS_ASCII_DIGIT_START, PRS_ASCII_DIGIT_END);
}

prs_ErrCode prs_createLetterRange(prs_Range *range, char c1, char c2, bool uppercase) {
    assert(range);

    range->uppercaseLetter = uppercase;

    if (uppercase) {
        c1 = tolower(c1);
        c2 = tolower(c2);
    }

    return createRange(range, c1, c2, PRS_ASCII_LETTER_START, PRS_ASCII_LETTER_END);
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

prs_ErrCode prs_extractRange(prs_Range *range, const char *input) {
    assert(range);
    assert(input);

    char c1, c2;

    int extractedElements = sscanf(input, "%c-%c", &c1, &c2);

    if (extractedElements != 2) {
        return PRS_INVALID_RANGE_PATTERN;
    }

    if (isalpha(c1) && isalpha(c2)) {
        return prs_createLetterRange(range, c1, c2, isupper(c1) && isupper(c2));
    }
    else if (isdigit(c1) && isdigit(c2)) {
        return prs_createDigitRange(range, c1, c2);
    }
    else {
        return PRS_INVALID_RANGE_PATTERN;
    }
}

prs_ErrCode prs_extractRanges(prs_Range **pRanges, const char *input, size_t length) {
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
        return PRS_INVALID_RANGE_PATTERN;
    }

    size_t rangesNumber = lengthWithoutSpaces / 3;
    prs_Range *ranges = calloc(rangesNumber, sizeof(*ranges));

    if (!ranges) {
        free(buffer);
        return -1;
    }

    const char *pos = input;
    size_t i;

    for (i = 0;i < rangesNumber;++i) {
        prs_ErrCode errCode = prs_extractRange(ranges + i, pos);

        if (errCode != PRS_OK) {
            free(buffer);
            free(ranges);
            return errCode;
        }
        pos += 3;
    }

    free(buffer);
    *pRanges = ranges;

    return i;
}

/**
 * Splits a string into prs_StringItem items and inserts them
 * into the given linked list.
 *
 * @param dest a pointer to a linked list
 * @param source the string to split
 * @param length length of the given string
 * @return number of extracted items or -1 if an error occurs
 */
static int splitItems(ll_LinkedList *dest, const char *source, size_t length) {
    ll_LinkedList rawItemList;
    ll_createLinkedList(&rawItemList, NULL);

    int result = str_splitItems(source, length, &rawItemList, ' ');

    ll_Iterator it = ll_createIterator(&rawItemList);

    while (ll_iteratorHasNext(&it)) {
        char *item = ll_iteratorNext(&it);

        prs_StringItem *stringItem = malloc(sizeof(*stringItem));
        stringItem->item = item;
        stringItem->line = stringItem->column = -1;

        ll_pushBack(dest, stringItem);
    }

    ll_freeLinkedList(&rawItemList, NULL);

    return result;
}

int prs_extractGrammarItems(const char *source, size_t length, ll_LinkedList *itemList) {
    assert(source);
    assert(itemList);

    // This buffer will hold a copy of the source
    char *buffer = malloc(length);

    if (!buffer) {
        return -1;
    }

    size_t sourcePos = 0;
    int extractedItems = 0;

    while (sourcePos < length) {
        *buffer = '\0';
        const char *currentPosPtr = source + sourcePos;

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

                int result = splitItems(itemList, buffer, blockLength);

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

                prs_StringItem *stringItem = malloc(sizeof(*stringItem));
                stringItem->item = stringBlock;
                stringItem->line = stringItem->column = -1;

                ll_pushBack(itemList, stringItem);
                extractedItems += 1;
            }

            size_t stringBlockLength = endBlockPos - currentPosPtr + 1;
            sourcePos += stringBlockLength;
        }
        else {
            ssize_t blockSize = str_removeMultipleSpaces(buffer, currentPosPtr, strlen(currentPosPtr));

            int result = splitItems(itemList, buffer, blockSize);

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
    int pwet = prs_splitDelimiters(&it, delimiters);

    return extractedItems + pwet;
}

ssize_t prs_readGrammar(FILE *stream, char **pBuffer) {
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

bool prs_computeItemsPosition(const char *source, ll_Iterator *it) {
    assert(source);
    assert(it);

    int line = 1;
    int column = 1;

    while (ll_iteratorHasNext(it)) {
        prs_StringItem *stringItem = ll_iteratorNext(it);

        char *start = strstr(source, stringItem->item);

        if (!start) {
            return false;
        }

        for (const char *curr = source;curr != start;++curr) {
            if (*curr == '\n') {
                column = 1;
                ++line;
            }
            else {
                ++column;
            }
        }

        stringItem->column = column;
        stringItem->line = line;

        source = start + strlen(stringItem->item);
        column += strlen(stringItem->item);
    }

    return true;
}

void prs_freeStringItem(prs_StringItem *stringItem) {
    if (stringItem) {
        free(stringItem->item);
        free(stringItem);
    }
}

prs_ErrCode prs_parseGrammarItems(fg_Grammar *g, ll_LinkedList *itemList) {
    assert(g);
    assert(itemList);

    ll_Iterator it = ll_createIterator(itemList);
    fg_Rule *entryRule = NULL;

    while (ll_iteratorHasNext(&it)) {
        prs_StringItem *stringItem = ll_iteratorNext(&it);

        if (strlen(stringItem->item) < 2 || *stringItem->item != '%' || !isalpha(stringItem->item[1])) {
            prs_setErrorState(stringItem);
            return PRS_UNKNOWN_ITEM;
        }

        if (isupper(stringItem->item[1])) {
            // it should be a token
            fg_Token *token = malloc(sizeof(*token));
            memset(token, 0, sizeof(*token));

            int errCode = fg_extractToken(token, &it, stringItem);

            if (errCode != PRS_OK) {
                fg_freeToken(token);
                free(token);
                prs_setErrorState(stringItem);
                return errCode;
            }

            if (ht_getValue(&g->tokens, token->name) != NULL) {
                fg_freeToken(token);
                free(token);
                prs_setErrorState(stringItem);

                return FG_TOKEN_EXISTS;
            }

            ht_insertElement(&g->tokens, token->name, token);
        }
        else {
            // it should be a rule
            fg_Rule *rule = malloc(sizeof(*rule));
            fg_createRule(rule);

            int errCode = fg_extractRule(rule, &it, stringItem);

            if (errCode != PRS_OK) {
                fg_freeRule(rule);
                free(rule);

                if (!prs_hasErrorState()) {
                    prs_setErrorState(stringItem);
                }
                return errCode;
            }

            if (ht_getValue(&g->rules, rule->name) != NULL) {
                fg_freeRule(rule);
                free(rule);
                prs_setErrorState(stringItem);
                return FG_RULE_EXISTS;
            }

            ht_insertElement(&g->rules, rule->name, rule);

            if (!entryRule) {
                entryRule = rule;
            }
        }
    }

    g->entry = entryRule;

    return PRS_OK;
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
            fg_Rule *refRule = ht_getValue(&resolverArg->g->rules, prItem->symbol->item);

            if (!refRule) {
                resolverArg->errCode = FG_UNKNOWN_RULE;
                prs_setErrorState(prItem->symbol);
                break;
            }

            prItem->rule = refRule;
        }
        else if (prItem->type == FG_TOKEN_ITEM) {
            fg_Token *refToken = ht_getValue(&resolverArg->g->tokens, prItem->symbol->item);

            if (!refToken) {
                resolverArg->errCode = FG_UNKNOWN_TOKEN;
                prs_setErrorState(prItem->symbol);
                break;
            }

            prItem->token = refToken;
        }
    }
}

prs_ErrCode prs_resolveSymbols(fg_Grammar *g) {
    ht_Iterator tokensIt;
    ht_createIterator(&tokensIt, &g->tokens);

    ht_Iterator rulesIt;
    ht_createIterator(&rulesIt, &g->rules);

    while (ht_iteratorHasNext(&tokensIt)) {
        ht_KVPair *pair = ht_iteratorNext(&tokensIt);
        fg_Token *token = pair->value;

        if (token->type == FG_REF_TOKEN) {
            struct fg_RefToken *refTokenValue = &token->value.refToken;
            fg_Token *refToken = ht_getValue(&g->tokens, refTokenValue->symbol->item);

            if (!refToken) {
                prs_setErrorState(refTokenValue->symbol);
                return FG_UNKNOWN_TOKEN;
            }

            refTokenValue->token = refToken;
        }
    }

    struct ResolverArg resolverArg = { .g = g, .errCode = PRS_OK };

    while (ht_iteratorHasNext(&rulesIt)) {
        ht_KVPair *pair = ht_iteratorNext(&rulesIt);
        fg_Rule *rule = pair->value;

        resolverArg.rule = rule;

        ll_forEachItem(&rule->productionRuleList, resolveProductionRulesSymbols, &resolverArg);

        if (resolverArg.errCode != PRS_OK) {
            return resolverArg.errCode;
        }
    }

    return PRS_OK;
}

int prs_splitDelimiters(ll_Iterator *it, const char *delimiters) {
    assert(it);
    assert(delimiters);

    int extractedItems = 0;

    while (ll_iteratorHasNext(it)) {
        prs_StringItem *stringItem = ll_iteratorNext(it);
        char *item = stringItem->item;

        if (*item == '`') {
            // If the item is a string block then it does not
            // require to be splitter
            continue;
        }

        char *ptr = strpbrk(item, delimiters);

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

            prs_StringItem *stringItem2 = malloc(sizeof(*stringItem2));
            stringItem2->item = newItem;
            stringItem2->column = stringItem->line = -1;

            // Inserts the new item into the list right after the current item
            ll_iteratorInsert(it, stringItem2);

            // Adds a null character at the position of the delimiter
            // We only keep the part that is before the delimiter
            // or the character itself in the case of ptr==item.
            *ptr = '\0';

            ++extractedItems;
        }
    }

    return extractedItems;
}

set_HashSet *prs_ruleFirsts(ht_Table *table, fg_Rule *rule) {
    assert(table);
    assert(rule);

    set_HashSet *set = ht_getValue(table, rule);

    if (set) {
        return set;
    }

    set = malloc(sizeof(*set));
    set_createSet(set, 10, NULL, NULL);
    ht_insertElement(table, rule, set);

    ll_Iterator prIt = ll_createIterator(&rule->productionRuleList);

    while (ll_iteratorHasNext(&prIt)) {
        ll_LinkedList *pr = ll_iteratorNext(&prIt);

        fg_PRItem *prItem = pr->front->data;

        switch (prItem->type) {
            case FG_TOKEN_ITEM:
                set_insertValue(set, prItem);
                break;
            case FG_RULE_ITEM:
                set_union(set, prs_ruleFirsts(table, prItem->rule));
                break;
        }
    }

    return set;
}
