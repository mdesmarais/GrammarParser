#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

struct ll_LinkedList;

typedef int ht_KeyComparator(const void*, const void*);
typedef void ht_KVPairDestructor(void*, void *);
typedef uint32_t ht_HashFunction(const void*);

typedef struct ht_KVPair {
    void *key;
    void *value;
} ht_KVPair;

typedef struct ht_Table {
    struct ll_LinkedList *buckets;
    size_t capacity;
    size_t size;
    ht_HashFunction *hashFunction;
    ht_KeyComparator *keyComparator;
    ht_KVPairDestructor *destructor;
} ht_Table;

/**
 * Initializes ht_Table structure with an initial capacity.
 *
 * If the allocation of the table' records failed then false will
 * be returned.
 *
 * The key comparator is not required : if it is null then
 * a pointer comparison will be used to check keys equality.
 *
 * The destructor is also not required : if it is null then
 * we assume that there is not point of freeing memory for each pair.
 * The destructor should free allocated memory for the pair key / value.
 *
 * @param table pointer to a table structure
 * @param capacity initial capacity of the table
 * @param hashFunction pointer to a function that computes a 256 bits hash
 * @param keyComparator pointer to a function that compares pair's key
 * @param destructor pointer to a destructor function
 * @return true if the initialization of the table succeed, otherwise false
 */
bool ht_createTable(ht_Table *table, size_t capacity, ht_HashFunction *hashFunction, ht_KeyComparator *keyComparator, ht_KVPairDestructor *destructor);

/**
 * Frees allocated memory in the given hash table.
 *
 * Fields capacity and size will be set to 0.
 *
 * @param table a pointer to a hash table structure
 */
void ht_freeTable(ht_Table *table);

/**
 * Inserts a pair (key/value) into the table.
 *
 * This hash table uses separate chaining to manage collisions.
 * If a pair will same hash already exists, then the new one will be added
 * at the end of the same bucket.
 *
 * If a pair with same key (according to the given key comparator)
 * already exists, then its value will be modified with the new one.
 *
 * @param table a pointer to a hash table structure
 * @param key
 * @param value
 */
void ht_insertElement(ht_Table *table, void *key, void *value);

/**
 * Removes a pair from the table.
 *
 * If no pair with the given key exists, then
 * nothing will be done.
 *
 * @param table a pointer to a hash table
 * @param key
 */
void ht_removeElement(ht_Table *table, const void *key);

/**
 * Computes a 256 bits hash value of the given string.
 *
 * Its based on the Jenkins hash function.
 *
 * The given string must be null terminated.
 *
 * @param data string to hash
 * @return computed hash value
 */
uint32_t ht_hashString(const void *data);

/**
 * Retrieves a value by using its key.
 *
 * It the given key does not exist, then a null
 * pointer will be returned.
 *
 * @param table a pointer to a hash table
 * @param key
 * @return pointer to the associated value or null
 */
void *ht_getValue(ht_Table *table, const void *key);

#endif // HASH_TABLE_H
