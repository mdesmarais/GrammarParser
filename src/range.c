#include "range.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include "string_utils.h"

#define ASCII_DIGIT_START 48
#define ASCII_DIGIT_END 57

#define ASCII_LETTER_START 97
#define ASCII_LETTER_END 122

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

    return createRange(range, n1, n2, ASCII_DIGIT_START, ASCII_DIGIT_END);
}

prs_ErrCode prs_createLetterRange(prs_Range *range, char c1, char c2, bool uppercase) {
    assert(range);

    range->uppercaseLetter = uppercase;

    if (uppercase) {
        c1 = tolower(c1);
        c2 = tolower(c2);
    }

    return createRange(range, c1, c2, ASCII_LETTER_START, ASCII_LETTER_END);
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
