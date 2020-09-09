#ifndef RANGE_H
#define RANGE_H

#include "parser_errors.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum prs_RangeQuantifier {
    PRS_NO_QUANTIFIER = 0,
    PRS_PLUS_QUANTIFIER,
    PRS_QMARK_QUANTIFIER,
    PRS_STAR_QUANTIFIER
} prs_RangeQuantifier;

typedef struct prs_Range {
    bool uppercaseLetter;
    uint8_t start;
    uint8_t end;
} prs_Range;

typedef struct prs_RangeArray {
    prs_Range *ranges;
    size_t size;
} prs_RangeArray;

/**
 * Creates a digit range.
 *
 * If n1 or n2 is not a valid digit, then PRS_INVALID_CHAR_RANGE
 * will be returned.
 * If n1 is greater than n2 then PRS_INVALID_RANGE will be returned.
 *
 * @param range a pointer to a range structure
 * @param n1 first digit
 * @param n2 second digit
 * @return PRS_OK if no error occured, otherwise a different error code
 */
prs_ErrCode prs_createDigitRange(prs_Range *range, char n1, char n2);

/**
 * Creates a letter range.
 *
 * If n1 or n2 is not a valid ascii letter, then PRS_INVALID_CHAR_RANGE
 * will be returned.
 * If n1 is greater than n2 (in terms of ascii value) then PRS_INVALID_RANGE
 * will be returned.
 *
 * @param range a pointer to a range structure
 * @param c1 first letter
 * @param c2 second letter
 * @param uppercase indicates whether or not the range should match an uppercase letter
 * @return PRS_OK if no error occured, otherwise a different error code
 */
prs_ErrCode prs_createLetterRange(prs_Range *range, char c1, char c2, bool uppercase);

/**
 * Checks if the given char is included in the given range.
 *
 * The ascii value of the char will be used to check if it is
 * in the range or not.
 *
 * @param range a pointer to a range structure
 * @param c a char
 * @param isLetter whether or not the char is a letter
 * @return true if the char is included in the given range, otherwise false
 */
bool prs_matchInRange(prs_Range *range, char c, bool isLetter);

/**
 * Extract a range from a given string.
 *
 * The string must be under the following form : x-y
 * where x and y represents two chars. The dash (-) is required.
 * If the string does not meet this requirement, then PRS_INVALID_RANGE_PATTERN
 * will be returned.
 *
 * @param range a pointer to a range
 * @param input source string
 * @return PRS_OK if not error occured, otherwise a different error code
 */
prs_ErrCode prs_extractRange(prs_Range *range, const char *input);

/**
 * Extracts several ranges from a given string.
 *
 * The string must only contain one or more patterns without square brackets.
 * If the string contains an invalid pattern, then PRS_INVALID_RANGE_PATTERN will
 * be returned.
 *
 * An array of prs_Range will be allocated in the range array structure.
 *
 * @param rangeArray a pointer to a range array
 * @param input source string
 * @param length length of the string
 * @return PRS_OK if not error occured, otherwise an error
 */
prs_ErrCode prs_extractRanges(prs_RangeArray *rangeArray, const char *input, size_t length);

/**
 * Frees allocated memory to store several ranges.
 *
 * The given pointer will not be freed.
 *
 * @param rangeArray a pointer to a range array
 */
void prs_freeRangeArray(prs_RangeArray *rangeArray);

#endif // RANGE_H
