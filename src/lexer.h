#ifndef LEXER_LEXER_H
#define LEXER_LEXER_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef enum lex_RangeQuantifier {
    LEX_PLUS_QUANTIFIER,
    LEX_QMARK_QUANTIFIER
} lex_RangeQuantifier;

typedef struct lex_Range {
    bool uppercaseLetter;
    const char *start;
    const char *end;
    lex_RangeQuantifier quantifier;
} lex_Range;

typedef enum lex_RetCode {
    LEXER_OK,
    LEXER_INVALID_CHAR_RANGE,
    LEXER_INVALID_RANGE,
    LEXER_INVALID_RANGE_PATTERN
} lex_RetCode;

#define LEXER_LOG_ERROR(code) log_error("%s -> %d", __FUNCTION__, (code))

struct ll_LinkedList;
struct ll_Iterator;

struct fg_Grammar;

int lex_createDigitRange(lex_Range *range, char n1, char n2);
int lex_createLetterRange(lex_Range *range, char c1, char c2, bool uppercase);
bool lex_matchInRange(lex_Range *range, char c, bool isLetter);

int lex_extractRange(lex_Range *range, const char *input);
int lex_extractRanges(lex_Range **pRanges, const char *input, size_t length);

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
int lex_extractGrammarItems(const char *source, size_t length, struct ll_LinkedList *itemList);

int lex_parseGrammarItems(struct fg_Grammar *g, struct ll_LinkedList *itemList);

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
ssize_t lex_readGrammar(FILE *stream, char **pBuffer);

/**
 * Separates items from any delimiter in the given array.
 *
 * Example : RULE= -> will be separated into 2 items : RULE, =
 * Separated items (ex: =) will be added into the list with
 * the given iterator. They will be placed between the original item
 * and the next one. The list order will be preserved.
 *
 * @param it iterator on the item list
 * @param delimiters array of delimiters
 * @param delimitersCount number of delimiters in the given array
 * @return number of added elements into the list
 */
int lex_splitDelimiters(struct ll_Iterator *it, const char *delimiters, size_t delimitersCount);

#endif //LEXER_LEXER_H
