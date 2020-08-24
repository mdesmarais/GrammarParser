#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef enum prs_RangeQuantifier {
    PRS_PLUS_QUANTIFIER,
    PRS_QMARK_QUANTIFIER
} prs_RangeQuantifier;

typedef struct prs_Range {
    bool uppercaseLetter;
    const char *start;
    const char *end;
} prs_Range;

typedef enum prs_RetCode {
    PRS_OK,
    PRS_INVALID_CHAR_RANGE,
    PRS_INVALID_RANGE,
    PRS_INVALID_RANGE_PATTERN,

    PRS_UNKNOWN_ITEM
} prs_RetCode;

struct ll_LinkedList;
struct ll_Iterator;

struct fg_Grammar;

int prs_createDigitRange(prs_Range *range, char n1, char n2);
int prs_createLetterRange(prs_Range *range, char c1, char c2, bool uppercase);
bool prs_matchInRange(prs_Range *range, char c, bool isLetter);

int prs_extractRange(prs_Range *range, const char *input);
int prs_extractRanges(prs_Range **pRanges, const char *input, size_t length);

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

int prs_parseGrammarItems(struct fg_Grammar *g, struct ll_LinkedList *itemList);

int prs_resolveSymbols(struct fg_Grammar *g);

/**
 * Reads a raw grammar from a stream
 *
 * This function requires a pointer to a buffer that
 * will be allocated depending on the size of the input.
 * The buffer should be NULL before calling this function.
 *
 * The caller has the responsability of freeing this buffer.
 *
 * If an allocation error occured, then -1 will be returned.
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
