#include "hash_table.h"

#include "linked_list.h"

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

static void pairDestructor(void *data, void *args) {
    ht_Table *table = args;
    ht_KVPair *pair = data;

    if (table->destructor) {
        table->destructor(pair->key, pair->value);
    }
    free(pair);
}

struct ComparatorArgument {
    const ht_Table *table;
    const void *key;
};

static int pairComparator(const void *data1, const void *data2) {
    const struct ComparatorArgument *arg = data1;
    const ht_KVPair *p2 = data2;

    if (arg->table->keyComparator) {
        return arg->table->keyComparator(arg->key, p2->key);
    }
    else {
        return arg->key == p2->key;
    }
}

bool ht_createTable(ht_Table *table, size_t capacity, ht_HashFunction *hashFunction, ht_KeyComparator *keyComparator, ht_KVPairDestructor *destructor) {
    assert(table);
    assert(hashFunction);

    ll_LinkedList *buckets = calloc(capacity, sizeof(*buckets));

    if (!buckets) {
        return false;
    }

    for (size_t i = 0;i < capacity;i++) {
        ll_createLinkedList(buckets + i, pairDestructor);
    }

    table->buckets = buckets;
    table->capacity = capacity;
    table->size = 0;
    table->hashFunction = hashFunction;
    table->keyComparator = keyComparator;
    table->destructor = destructor;

    return true;
}

void ht_freeTable(ht_Table *table) {
    if (table) {
        for (size_t i = 0;i < table->capacity;++i) {
            ll_LinkedList *buckets = table->buckets + i;

            ll_freeLinkedList(buckets, table);
        }

        free(table->buckets);
        table->buckets = NULL;
        table->capacity = table->size = 0;
    }
}

static ll_LinkedList *getBucket(ht_Table *table, const void *key) {
    assert(table);
    assert(key);

    uint32_t hash = table->hashFunction(key);
    size_t index = hash % table->capacity;

    return table->buckets + index;
}

static ht_KVPair *getPairByKey(ht_Table *table, ll_LinkedList *bucket, const void *key) {
    assert(table);
    assert(bucket);
    assert(key);

    struct ComparatorArgument arg = { .table = table, .key = key };
    return ll_findItem(bucket, &arg, pairComparator);
}

void ht_insertElement(ht_Table *table, void *key, void *value) {
    assert(table);
    assert(key);
    assert(value);

    ll_LinkedList *bucket = getBucket(table, key);
    ht_KVPair *existingPair = getPairByKey(table, bucket, key);

    if (existingPair) {
        existingPair->value = value;
    }
    else {
        ht_KVPair *pair = malloc(sizeof(*pair));
        pair->key = key;
        pair->value = value;
        ll_pushBack(bucket, pair);
        ++table->size;
    }
}



void ht_removeElement(ht_Table *table, const void *key) {
    assert(table);
    assert(key);

    ll_LinkedList *bucket = getBucket(table, key);

    struct ComparatorArgument arg = { .table = table, .key = key };

    if (ll_removeItem(bucket, &arg, pairComparator, table)) {
        --table->size;
    }
}

uint32_t ht_hashString(const void *data) {
    uint32_t value = 0;

    while (*((char*) data) != '\0') {
        value += *((char*) data);
        value += value << 10;
        value ^= value >> 6;
        ++data;
    }

    value += value << 3;
    value ^= value >> 11;
    value += value << 15;

    return value;
}

void *ht_getValue(ht_Table *table, const void *key) {
    assert(table);
    assert(key);

    ll_LinkedList *bucket = getBucket(table, key);
    ht_KVPair *pair = getPairByKey(table, bucket, key);

    return (pair) ? pair->value : NULL;
}

static size_t firstNonEmptyBucketIndex(ll_LinkedList *buckets, size_t offset, size_t limit) {
    size_t index = offset;

    while (index < limit && buckets[index].size == 0) {
        ++index;
    }

    return index;
}

void ht_createIterator(ht_Iterator *it, ht_Table *table) {
    assert(it);
    assert(table);
    assert(table->capacity > 0);

    // Looking for the first non empty bucket
    size_t index = firstNonEmptyBucketIndex(table->buckets, 0, table->capacity);

    it->table = table;
    it->bucketIndex = index;

    if (index < table->capacity) {
        ll_initIterator(&it->internalIt, table->buckets + index);
    }
}

bool ht_iteratorHasNext(ht_Iterator *it) {
    assert(it);

    return it->bucketIndex < it->table->capacity && ll_iteratorHasNext(&it->internalIt);
}

ht_KVPair *ht_iteratorNext(ht_Iterator *it) {
    assert(it);
    assert(ht_iteratorHasNext(it));

    ht_KVPair *current = ll_iteratorNext(&it->internalIt);

    if (!ll_iteratorHasNext(&it->internalIt)) {
        it->bucketIndex = firstNonEmptyBucketIndex(it->table->buckets, it->bucketIndex + 1, it->table->capacity);

        if (it->bucketIndex < it->table->capacity) {
            ll_initIterator(&it->internalIt, it->table->buckets + it->bucketIndex);
        }
    }

    return current;
}