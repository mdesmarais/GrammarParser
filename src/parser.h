#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "parser_errors.h"

typedef struct prs_StringItem {
    char *item;
    int line;
    int column;
} prs_StringItem;

typedef enum prs_RangeQuantifier {
    PRS_PLUS_QUANTIFIER,
    PRS_QMARK_QUANTIFIER
} prs_RangeQuantifier;

typedef struct prs_Range {
    bool uppercaseLetter;
    const char *start;
    const char *end;
} prs_Range;

struct ll_LinkedList;
struct ll_Iterator;

struct fg_Grammar;

prs_ErrCode prs_createDigitRange(prs_Range *range, char n1, char n2);
prs_ErrCode prs_createLetterRange(prs_Range *range, char c1, char c2, bool uppercase);
bool prs_matchInRange(prs_Range *range, char c, bool isLetter);

prs_ErrCode prs_extractRange(prs_Range *range, const char *input);
prs_ErrCode prs_extractRanges(prs_Range **pRanges, const char *input, size_t length);

/**
 * Extracts items from a given raw grammar.
 *
 * Available items are : ";", "=", "|", "+", "?", token name, rule items, ranges.
 * All extracted items are null terminated.
 *
 * @param source
 * @param length length of the source
 * @param itemList list that will receive extracted items
 * @return number of extracted items
 */
int prs_extractGrammarItems(const char *source, size_t length, struct ll_LinkedList *itemList);

/**
 * Computes position of each item in the list in the given source.
 *
 * The source must be a null terminated string.
 * If an item is not found in the string, then the function will return false.
 * This functions updates fields line and column of prs_StringItem structure.
 *
 * @param source a null terminated string
 * @param it pointer to an iterator on a prs_String list
 * @return true if all items have been found in the string, otherwise false
 */
bool prs_computeItemsPosition(const char *source, struct ll_Iterator *it);

void prs_freeStringItem(prs_StringItem *stringItem);

prs_ErrCode prs_parseGrammarItems(struct fg_Grammar *g, struct ll_LinkedList *itemList);

prs_ErrCode prs_resolveSymbols(struct fg_Grammar *g);

/**
 * Reads a raw grammar from a stream
 *
 * This function requires a pointer to a buffer that
 * will be allocated depending on the size of the input.
 * The buffer should be NULL before calling this function.
 *
 * The caller has the responsability of freeing this buffer.
 *
 * If an allocation error occurs, then -1 will be returned.
 *
 * @param stream input stream
 * @param pBuffer pointer to a NULL buffer
 * @return length of the extracted content
 */
ssize_t prs_readGrammar(FILE *stream, char **pBuffer);

/**
 * Separates items from any delimiter in the given string.
 *
 * Example : RULE= -> will be separated into 2 items : RULE, =
 * Separated items (ex: =) will be added into the list with
 * the given iterator. They will be placed between the original item
 * and the next one. The list order will be preserved.
 *
 * The delimiters string must be null terminated.
 *
 * @param it iterator on the item list
 * @param delimiters string of delimiters
 * @return number of added elements into the list
 */
int prs_splitDelimiters(struct ll_Iterator *it, const char *delimiters);

#endif // PARSER_H
