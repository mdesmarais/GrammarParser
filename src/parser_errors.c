#include "parser_errors.h"

#include "parser.h"

#include <assert.h>
#include <string.h>

static prs_ErrorState _currentState;

static const char *errorMessages[] = {
        "No error",
        "Invalid char range",
        "Invalid range",
        "Invalid range pattern",

        "Invalid token",
        "Missing end marker for token",
        "Missing value for token",
        "Invalid value for token",
        "Unknown value type for token",
        "Self referencing is forbidden for token",

        "Empty rule",
        "Invalid rule",
        "Missing end marker for rule",
        "Missing value for rule",

        "Unknown token symbol",
        "Unknown rule symbol",
        "Unknown production rule item",

        "Empty production rule",
        "Unknown item type"
};

size_t prs_getErrorMessage(char *buffer, size_t capacity, prs_ErrCode errCode) {
    assert(buffer);
    assert(capacity > 0);

    assert(errCode >= 0 && errCode < PRS_MAX_CODE_NUMBER);

    // We need one char for the null character
    --capacity;

    const char *msg = errorMessages[errCode];
    size_t length = strlen(msg);

    size_t copiedChars = (capacity > length) ? length : capacity;
    strncpy(buffer, msg, copiedChars);

    size_t remainingCapacity = capacity - copiedChars;

    size_t totalLength = copiedChars;
    char *currentBufferPos = buffer + copiedChars;

    if (_currentState.stringItem) {
        prs_StringItem *stringItem = _currentState.stringItem;
        totalLength += snprintf(currentBufferPos, remainingCapacity, " %s (%d:%d)",
                                stringItem->item, stringItem->line, stringItem->column);
    }

    *(buffer + totalLength) = '\0';

    return totalLength;
}

bool prs_hasErrorState() {
    return _currentState.stringItem != NULL;
}

void prs_setErrorState(prs_StringItem *stringItem) {
    assert(stringItem);

    _currentState.stringItem = stringItem;
}
