#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stdio.h>
#include <string.h>

struct ll_LinkedList;

/**
 * Removes all whitespaces from a given source string.
 *
 * The result will be written into the destination ptr.
 * It should be large enough to hold the new string.
 *
 * @param dest output char pointer
 * @param source source char pointer
 * @param length length of the source string
 * @return length of the new string in dest
 */
size_t str_removeWhitespaces(char *dest, const char *source, size_t length);

/**
 * Removes all consecutives spaces from a given source string.
 *
 * If the given string contains two or more consecutive spaces,
 * then they will be removed : only one space will be left
 * in the destination pointer.
 *
 * The result will be written into the destination ptr.
 * It should be large enough to hold the new string.
 *
 * @param dest output char pointer
 * @param source source char pointer
 * @param length length of the source string
 * @return length of the new string in dest
 */
size_t str_removeMultipleSpaces(char *dest, const char *source, size_t length);

/**
 * Splits string into items that are separated by the given separator.
 *
 * Extracted items are inserted into the given list.
 * An item is extracted only if it is not empty.
 *
 * @param source source string
 * @param length length of the string
 * @param itemList linked list that will receive items
 * @param delimiters
 * @param delimitersNumber
 * @return number of extracted elements or -1 if an error occured
 */
int str_splitItems(const char *source, size_t length, struct ll_LinkedList *itemList, char separator);

#endif //STRING_UTILS_H
