#include "parser.h"

#include "collections/hash_table.h"
#include "collections/linked_list.h"
#include "formal_grammar.h"
#include "log.h"
#include "string_utils.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

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

bool prs_stringItemEquals(prs_StringItem *si1, prs_StringItem *si2) {
    if (si1 == si2) {
        return true;
    }

    return si1->line == si2->line && si1->column == si2->column && strcmp(si1->item, si2->item) == 0;
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

            prItem->value.rule = refRule;
        }
        else if (prItem->type == FG_TOKEN_ITEM) {
            fg_Token *refToken = ht_getValue(&resolverArg->g->tokens, prItem->symbol->item);

            if (!refToken) {
                resolverArg->errCode = FG_UNKNOWN_TOKEN;
                prs_setErrorState(prItem->symbol);
                break;
            }

            prItem->value.token = refToken;
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

static void parserItemDestructor(prs_ParserItem *parserItem) {
    if (parserItem) {
        prs_freeParserItem(parserItem);
        free(parserItem);
    }
}

void prs_freeParserItem(prs_ParserItem *parserItem) {
    if (parserItem) {
        switch (parserItem->type) {
            case PRS_RANGE_ITEM:
                free(parserItem->value.rangeArray.ranges);
                break;
            case PRS_STRING_ITEM:
                free(parserItem->value.string);
                break;
        }
        memset(&parserItem->value, 0, sizeof(parserItem->value));
    }
}
