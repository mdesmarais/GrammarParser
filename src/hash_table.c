#include "hash_table.h"

#include "linked_list.h"

#include <assert.h>
#include <stdlib.h>

bool ht_createTable(ht_Table *table, size_t capacity, ht_HashFunction *hashFunction, ht_KeyComparator *keyComparator) {
    assert(table);
    assert(hashFunction);

    ll_LinkedList *buckets = calloc(capacity, sizeof(*buckets));

    if (!buckets) {
        return false;
    }

    table->buckets = buckets;
    table->capacity = capacity;
    table->size = 0;
    table->hashFunction = hashFunction;
    table->keyComparator = keyComparator;

    return true;
}

void ht_freeTable(ht_Table *table, ht_KVPairDestructor *destructor) {
    if (table) {
        for (size_t i = 0;i < table->capacity;++i) {
            ll_LinkedList *buckets = table->buckets + i;

            if (destructor) {
                ll_freeLinkedList(buckets, destructor);
            }
            else {
                ll_freeLinkedList(buckets, NULL);
            }
        }

        free(table->buckets);
        table->capacity = table->size = 0;
    }
}

void ht_createPair(ht_KVPair *pair, void *key, void *value) {
    assert(pair);
    assert(key);
    assert(value);

    pair->key = key;
    pair->value = value;
}

struct Pwet {
    ht_Table *table;
    char *key;
};

static int pairComparator(const void *data1, const void *data2) {
    const struct Pwet *arg = data1;
    const ht_KVPair *p2 = data2;

    if (arg->table->keyComparator) {
        return arg->table->keyComparator(arg->key, p2->key);
    }
    else {
        return arg->key == p2->key;
    }
}

void ht_insertPair(ht_Table *table, ht_KVPair *pair) {
    assert(table);
    assert(pair);

    uint32_t hash = table->hashFunction(pair->key);
    size_t index = hash % table->capacity;

    ll_LinkedList *bucket = table->buckets + index;

    struct Pwet arg = { .table = table, .key = pair->key };

    ht_KVPair *existingPair = ll_findItem(bucket, &arg, pairComparator);

    if (existingPair) {
        existingPair->value = pair->value;
    }
    else {
        ll_pushBack(bucket, pair);
        ++table->size;
    }
}

uint32_t ht_hashString(void *data) {
    return 0;
}
