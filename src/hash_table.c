#include "hash_table.h"

#include "linked_list.h"

#include <assert.h>
#include <stdlib.h>

bool ht_createTable(ht_Table *table, size_t capacity, ht_HashFunction *hashFunction, ht_KeyComparator *keyComparator, ht_KVPairDestructor *destructor) {
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
    table->destructor = destructor;

    return true;
}

void ht_freeTable(ht_Table *table) {
    if (table) {
        for (size_t i = 0;i < table->capacity;++i) {
            ll_LinkedList *buckets = table->buckets + i;

            if (table->destructor) {
                ll_freeLinkedList(buckets, table->destructor);
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
    const char *key;
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

    struct Pwet arg = { .table = table, .key = key };
    return ll_findItem(bucket, &arg, pairComparator);
}

void ht_insertPair(ht_Table *table, ht_KVPair *pair) {
    assert(table);
    assert(pair);

    ll_LinkedList *bucket = getBucket(table, pair->key);
    ht_KVPair *existingPair = getPairByKey(table, bucket, pair->key);

    if (existingPair) {
        existingPair->value = pair->value;
    }
    else {
        ll_pushBack(bucket, pair);
        ++table->size;
    }
}

uint32_t ht_hashString(const void *data) {
    uint32_t value = 0;

    while (*((char*) data) != '\0') {
        value += *((char*) data);
        value += value << 10;
        value ^= value >> 6;
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

void ht_removeElement(ht_Table *table, const void *key) {
    assert(table);
    assert(key);

    ll_LinkedList *bucket = getBucket(table, key);

    struct Pwet arg = { .table = table, .key = key };
    if (ll_removeItem(bucket, &arg, pairComparator, table->destructor)) {
        --table->size;
    }
}
