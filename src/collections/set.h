#ifndef SET_H
#define SET_H

#include "hash_table.h"

#include <stdbool.h>
#include <string.h>

typedef void set_ElementDestructor(void*);
typedef void set_CopyFactory(void **pDst, const void *src);

typedef struct set_HashSet {
    ht_Table table;
    set_CopyFactory *copyFactory;
    size_t size;
} set_HashSet;

typedef struct set_Iterator {
    ht_Iterator tableIt;
    set_HashSet *set;
} set_Iterator;

/**
 * Creates a new set.
 *
 * copyFactory and destructor pointers can be NULL.
 *
 * @param set a pointer to a set
 * @param initialCapacity initial capacity
 * @param hashFunction pointer to a hash function
 * @param comparator pointer to a comparator function
 * @param copyFactory pointer to a function that will create a copy of a value
 * @param destructor pointer to a destructor function
 */
void
set_createSet(set_HashSet *set, size_t initialCapacity, ht_HashFunction *hashFunction, ht_KeyComparator *comparator,
              set_CopyFactory *copyFactory, set_ElementDestructor *destructor);

/**
 * Frees allocated memory for the given set.
 *
 * The given pointer will not be freed.
 *
 * @param set a pointer to a set
 */
void set_freeSet(set_HashSet *set);

/**
 * Inserts a value into the given set.
 *
 * @param set a pointer to a set
 * @param value the value to insert
 */
void set_insertValue(set_HashSet *set, void *value);

/**
 * Checks if a set contains a given value.
 *
 * The comparator function (given in the contructor) will be used to do
 * the comparison. If it is null, then an address comparison will be done.
 *
 * @param set a pointer to a set
 * @param value the required value
 * @return true if the set contains the value, otherwise false
 */
bool set_contains(set_HashSet *set, void *value);

/**
 * Inserts all items from s2 into s1.
 *
 * s1 and s2 must have the same copy factory pointer.
 *
 * @param s1 a pointer to the destination set
 * @param s2 a pointer to a set
 */
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
