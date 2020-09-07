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

prs_ErrCode prs_createDigitRange(prs_Range *range, char n1, char n2);
prs_ErrCode prs_createLetterRange(prs_Range *range, char c1, char c2, bool uppercase);
bool prs_matchInRange(prs_Range *range, char c, bool isLetter);

prs_ErrCode prs_extractRange(prs_Range *range, const char *input);
prs_ErrCode prs_extractRanges(prs_Range **pRanges, const char *input, size_t length);

#endif // RANGE_H
