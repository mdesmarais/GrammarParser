#ifndef HASH_H
#define HASH_H

/**
 * @file
 * Defines functions to hash objects.
 * It also defines MurmurHash3 functions.
 */

#include <stdint.h>
#include <string.h>

struct fg_PRItem;
struct ll_LinkedList;
struct prs_ParserItem;
struct prs_StringItem;

/**
 * Computes a hash value for the given production rule.
 *
 * @param pr pointer to a production rule
 * @return a hash value
 */
uint32_t hashProductionRule(struct ll_LinkedList *pr);

/**
 * Computes a hash value for the given production rule item.
 *
 * @param prItem a pointer to a production rule item
 * @return a hash value
 */
uint32_t hashPRItem(const struct fg_PRItem *prItem);

/**
 * Computes a hash value for the given parser item.
 *
 * @param parserItem a pointer to a parser item
 * @return a hash value
 */
uint32_t prs_hashParserItem(const struct prs_ParserItem *parserItem);

/**
 * Computes a hash value for a null terminated string.
 *
 * @param string a null terminated string
 * @return a hash value
 */
uint32_t hashString(const char *string);

/**
 * Computes a hash value for the given string item.
 *
 * @param stringItem pointer to a string item
 * @return a hash value
 */
uint32_t hashStringItem(const struct prs_StringItem *stringItem);

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
