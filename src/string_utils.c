#include "string_utils.h"

#include "collections/linked_list.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

int str_numberLength(int n) {
    int length = 0;

    if (n < 0) {
        // If the number is negative then we need to take
        // account of the minus sign.
        ++length;
        n = -n;
    }

    while (n >= 10) {
        ++length;
        n /= 10;
    }

    return length + 1;
}

size_t str_removeWhitespaces(char *dest, const char *source, size_t length) {
    assert(dest);
    assert(source);

    const char *end = source + length;

    const char *ptr1 = source;
    char *ptr2 = dest;

    while (ptr1 != end) {
        if (*ptr1 == '\0') {
            break;
        }
        if (!isspace(*ptr1)) {
            *ptr2 = *ptr1;
            ++ptr2;
        }
        ++ptr1;
    }

    return ptr2 - dest;
}

size_t str_removeMultipleSpaces(char *dest, const char *source, size_t length) {
    assert(dest);
    assert(source);

    const char *end = source + length;

    const char *ptr1 = source;
    char *ptr2 = dest;

    while (ptr1 != end) {
        while (ptr1 != end && isspace(*ptr1) && (ptr1 + 1 != end && isspace(*(ptr1 + 1)))) {
            ++ptr1;
        }

        if (isspace(*ptr1)) {
            *ptr2++ = ' ';
            ++ptr1;
        }
        else {
            *ptr2++ = *ptr1++;
        }
    }

    *ptr2 = '\0';

    return ptr2 - dest;
}

static char *extractItem(const char *source, size_t itemLength) {
    assert(source);

    char *item = malloc(itemLength + 1);

    if (!item) {
        return NULL;
    }

    memcpy(item, source, itemLength);
    item[itemLength] = '\0';

    return item;
}

int str_splitItems(const char *source, size_t length, struct ll_LinkedList *itemList, char separator) {
    assert(source);
    assert(itemList);

    size_t initialListSize = itemList->size;

    size_t currentPos = 0;
    char *separatorPtr = NULL;

    while (currentPos < length && (separatorPtr = memchr(source + currentPos, separator, length - currentPos)) != NULL) {
        size_t separatorPos = separatorPtr - source;
        size_t itemLength = separatorPos - currentPos;

        if (itemLength <= 0) {
            currentPos  = separatorPos + 1;
            continue;
        }

        char *item = extractItem(source + currentPos, itemLength);

        if (!item) {
            return -1;
        }

        ll_pushBack(itemList, item);

        currentPos = separatorPos + 1;
    }

    if (currentPos < length) {
        size_t itemLength = length - currentPos;
        char *item = extractItem(source + currentPos, itemLength);

        if (!item) {
            return -1;
        }

        ll_pushBack(itemList, item);
    }

    return itemList->size - initialListSize;
}
