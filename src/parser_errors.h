#ifndef PARSER_ERRORS_H
#define PARSER_ERRORS_H

/**
 * @file
 * Defines errors code.
 */

#include <stdbool.h>
#include <stdio.h>

struct prs_StringItem;

typedef enum prs_ErrCode {
    PRS_OK = 0,
    PRS_INVALID_CHAR_RANGE,
    PRS_INVALID_RANGE,
    PRS_INVALID_RANGE_PATTERN,

    FG_TOKEN_INVALID,
    FG_TOKEN_MISSING_END,
    FG_TOKEN_MISSING_VALUE,
    FG_TOKEN_INVALID_VALUE,
    FG_TOKEN_UNKNOWN_VALUE_TYPE,
    FG_TOKEN_SELF_REF,
    FG_TOKEN_EXISTS,

    FG_RULE_EMPTY,
    FG_RULE_INVALID,
    FG_RULE_MISSING_END,
    FG_RULE_MISSING_VALUE,
    FG_RULE_EXISTS,

    FG_UNKNOWN_TOKEN,
    FG_UNKNOWN_RULE,
    PRS_UNKNOWN_ITEM,

    FG_PR_EMPTY,

    FG_PRITEM_UNKNOWN_TYPE,

    FG_STRING_BLOCK_MISSING_END,
    FG_STRING_BLOCK_EMPTY,

    PRS_MAX_CODE_NUMBER
} prs_ErrCode;

typedef struct prs_ErrorState {
    struct prs_StringItem *stringItem;
} prs_ErrorState;

/**
 * Copies the message that corresponds to the current error state into a buffer.
 *
 * The buffer's capacity must not be null.
 *
 * The error state's code must be a value defined in
 * the prs_ErrCode enum.
 *
 * The message will be null terminated.
 * No free is be needed.
 *
 * @param buffer a buffer
 * @param capacity buffer capacity
 * @param errCode code of the error
 * @return length of the message
 */
size_t prs_getErrorMessage(char *buffer, size_t capacity, prs_ErrCode errCode);

bool prs_hasErrorState();
void prs_setErrorState(struct prs_StringItem *stringItem);

#endif // PARSER_ERRORS_H
