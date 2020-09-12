#ifndef HELPERS_HPP
#define HELPERS_HPP

extern "C" {
#include <collections/set.h>
}

#include <string>
#include <vector>

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

template <class T>
T *firstSetItem(set_HashSet *set) {
    set_Iterator it;
    set_createIterator(&it, set);

    return (T*) set_iteratorNext(&it);
}

#endif // HELPERS_HPP
