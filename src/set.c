#include "set.h"

#include "hash_table.h"

#include <assert.h>

static int SENTINEL = 1;

void set_createSet(set_HashSet *set, size_t initialCapacity, ht_KeyComparator *comparator, set_ElementDestructor *destructor) {
    ht_createTable(set, initialCapacity, ht_hashString, comparator, (ht_KVPairDestructor*) destructor);
}

void set_freeSet(set_HashSet *set) {
    ht_freeTable(set);
}

void set_insertValue(set_HashSet *set, void *value) {
    assert(set);
    assert(value);

    ht_insertElement(set, value, &SENTINEL);
}

bool set_contains(set_HashSet *set, const void *value) {
    return ht_getValue(set, value) != NULL;
}
