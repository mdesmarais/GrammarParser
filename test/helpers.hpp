#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <string>
#include <vector>

// Macro used to get a char from a prs_StringItem with an iterator
#define nextStringItem(it) ((prs_StringItem*) ll_iteratorNext((it)))->item

struct ll_LinkedList;

/**
 * Fills a list of prs_StringItem from a vector of C++ strings.
 *
 * For each string, a new prs_StringItem is allocated with default columns
 * and lines (-1). The corresponding string will be copîed to an allocated buffer.
 * The new item will be inserted into the given list, it should be freed with
 * prs_freeStringItem.
 *
 * @param itemList the list to fill
 * @param items list of strings
 */
void fillItemList(ll_LinkedList *itemList, const std::vector<std::string> &items);

#endif // HELPERS_HPP
