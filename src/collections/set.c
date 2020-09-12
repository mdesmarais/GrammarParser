#include "set.h"

#include "hash_table.h"

#include <assert.h>

static int SENTINEL = 1;

void
set_createSet(set_HashSet *set, size_t initialCapacity, ht_HashFunction *hashFunction, ht_KeyComparator *comparator,
              set_CopyFactory *copyFactory, set_ElementDestructor *destructor) {
    ht_createTable(&set->table, initialCapacity, hashFunction, comparator, (ht_KVPairDestructor*) destructor);
    set->copyFactory = copyFactory;
    set->size = 0;
}

void set_freeSet(set_HashSet *set) {
    ht_freeTable(&set->table);
}

void set_insertValue(set_HashSet *set, void *value) {
    assert(set);
    assert(value);

    ht_insertElement(&set->table, value, &SENTINEL);
    set->size = set->table.size;
}

bool set_contains(set_HashSet *set, void *value) {
    return ht_getValue(&set->table, value) != NULL;
}

void set_union(set_HashSet *s1, set_HashSet *s2) {
    assert(s1);
    assert(s2);
    assert(s1->copyFactory == s2->copyFactory);

    if (s2->table.size == 0) {
        return;
    }

    set_Iterator it;
    set_createIterator(&it, s2);

    while (set_iteratorHasNext(&it)) {
        void *value = set_iteratorNext(&it);

        if (s2->copyFactory) {
            void *copy = NULL;
            s2->copyFactory(&copy, value);
            assert(copy);

            set_insertValue(s1, copy);
        }
        else {
            set_insertValue(s1, value);
        }
    }
}

void set_createIterator(set_Iterator *it, set_HashSet *set) {
    it->set = set;
    ht_createIterator(&it->tableIt, &set->table);
}

bool set_iteratorHasNext(set_Iterator *it) {
    return ht_iteratorHasNext(&it->tableIt);
}

void *set_iteratorNext(set_Iterator *it) {
    ht_KVPair *pair = ht_iteratorNext(&it->tableIt);
    return pair->key;
}
