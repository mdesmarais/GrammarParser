#ifndef SET_H
#define SET_H

#include "hash_table.h"

#include <stdbool.h>
#include <string.h>

typedef struct ht_Table set_HashSet;
typedef struct ht_Iterator set_Iterator;
typedef void set_ElementDestructor(void*);

void set_createSet(set_HashSet *set, size_t initialCapacity, ht_HashFunction *hashFunction, ht_KeyComparator *comparator, set_ElementDestructor *destructor);
void set_freeSet(set_HashSet *set);

void set_insertValue(set_HashSet *set, void *value);
bool set_contains(set_HashSet *set, void *value);

void set_union(set_HashSet *s1, set_HashSet *s2);

/**
 * Creates an iterator on a given hash set.
 *
 * The hash set must have a positive capacity.
 *
 * @param it a pointer to the iterator to create
 * @param table a pointer to a hash set
 */
void set_createIterator(set_Iterator *it, set_HashSet *set);

/**
 * Checks if there is another element at the current iterator's position.
 *
 * @param it a pointer to an iterator
 * @return true if there is one more element, otherwise false
 */
bool set_iteratorHasNext(set_Iterator *it);

/**
 * Gets the value at the current iterator's position.
 *
 * The iterator must not be at the end !
 *
 * @param it a pointer to an iterator
 * @return the current value
 */
void *set_iteratorNext(set_Iterator *it);

#endif //SET_H
