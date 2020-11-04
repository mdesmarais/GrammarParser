#ifndef HASH_H
#define HASH_H

/**
 * @file
 * Defines functions to hash objects.
 * It also defines MurmurHash3 functions.
 */

#include <stdint.h>
#include <string.h>

/**
 * Computes a hash value for a null terminated string.
 *
 * @param string a null terminated string
 * @return a hash value
 */
uint32_t hashString(const char *string);

/**
 * Computes a 128 bits hash value for the given key.
 *
 * It uses the murmuhash3 algorithm.
 *
 * @param key a pointer to a key to hash
 * @param length number of bytes to hash
 * @return a hash value
 */
uint32_t murmurhash3_32(const void *key, size_t length);

#endif // HASH_H
