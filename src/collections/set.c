#include "set.h"

#include "hash_table.h"

#include <assert.h>

static int SENTINEL = 1;

void set_createSet(set_HashSet *set, size_t initialCapacity, ht_HashFunction *hashFunction, ht_KeyComparator *comparator, set_ElementDestructor *destructor) {
    ht_createTable(set, initialCapacity, hashFunction, comparator, (ht_KVPairDestructor*) destructor);
}

void set_freeSet(set_HashSet *set) {
    ht_freeTable(set);
}

void set_insertValue(set_HashSet *set, void *value) {
    assert(set);
    assert(value);

    ht_insertElement(set, value, &SENTINEL);
}

bool set_contains(set_HashSet *set, void *value) {
    return ht_getValue(set, value) != NULL;
}

void set_union(set_HashSet *s1, set_HashSet *s2) {
    assert(s1);
    assert(s2);

    if (s2->size == 0) {
        return;
    }

    set_Iterator it;
    set_createIterator(&it, s2);

    while (set_iteratorHasNext(&it)) {
        void *value = set_iteratorNext(&it);

        set_insertValue(s1, value);
    }
}

void set_createIterator(set_Iterator *it, set_HashSet *set) {
    ht_createIterator(it, set);
}

bool set_iteratorHasNext(set_Iterator *it) {
    return ht_iteratorHasNext(it);
}

void *set_iteratorNext(set_Iterator *it) {
    ht_KVPair *pair = ht_iteratorNext(it);
    return pair->key;
}
