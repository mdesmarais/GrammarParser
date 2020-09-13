#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "collections/set.h"
#include "parser_errors.h"
#include "range.h"

typedef struct prs_StringItem {
    char *item;
    int line;
    int column;
} prs_StringItem;

typedef enum prs_ParserItemType {
    PRS_RANGE_ITEM,
    PRS_STRING_ITEM
} prs_ParserItemType;

union prs_ParserItemValue {
    prs_RangeArray rangeArray;
    char *string;
};

typedef struct prs_ParserItem {
    prs_ParserItemType type;
    union prs_ParserItemValue value;
} prs_ParserItem;

struct ll_LinkedList;
struct ll_Iterator;

struct fg_Grammar;
struct fg_PRItem;
struct fg_Rule;

/**
 * Extracts items from a given raw grammar.
 *
 * Available items are : ";", "=", "|", "+", "?", token name, rule items, ranges.
 * All extracted items are null terminated.
 *
 * @param source
 * @param length length of the source
 * @param itemList list that will receive extracted items
 * @return number of extracted items
 */
int prs_extractGrammarItems(const char *source, size_t length, struct ll_LinkedList *itemList);

/**
 * Computes position of each item in the list in the given source.
 *
 * The source must be a null terminated string.
 * If an item is not found in the string, then the function will return false.
 * This functions updates fields line and column of prs_StringItem structure.
 *
 * @param source a null terminated string
 * @param it pointer to an iterator on a prs_String list
 * @return true if all items have been found in the string, otherwise false
 */
bool prs_computeItemsPosition(const char *source, struct ll_Iterator *it);

/**
 * Frees allocated memory for the given string item.
 *
 * The given pointer will not be freed.
 *
 * @param stringItem a pointer to a string item
 */
void prs_freeStringItem(prs_StringItem *stringItem);

/**
 * Transforms a list of items into a grammar structure.
 *
 * A grammar is made by one or more rule and token definitions.
 *
 * If a prs_StringItem does not correspond to a rule or a token, PrS_UNKNOWN_ITEM will
 * be returned.
 *
 * A rule definition starts with "%" followed by a lowercase letter. If the rule
 * already exists then FG_RULE_EXISTS will be returned.
 * Other error codes can be returned by {@link fg_extractRule}.
 *
 * A token definition starts with "%" followed by an uppercase letter. If the token
 * already exists then FG_TOKEN_EXISTS will be returned.
 * Other error codes can be returned by {@link fg_extractToken}.
 *
 * @param g a pointer to the grammar structure
 * @param itemList a pointer to a list of prs_StringItem
 * @return PRS_OK if not error occurs, otherwise a different error code
 */
prs_ErrCode prs_parseGrammarItems(struct fg_Grammar *g, struct ll_LinkedList *itemList);

/**
 * Resolves symbol references.
 *
 * Symbol references occur in token and rule declarations : a rule can references a token or another rule,
 * and a token can references another token.
 *
 * If a token or a rule references an unknown token then FG_UNKNOWN_TOKEN will be returned.
 * If a rule references an unknown rule then FG_UNKNOWN_RULE will be returned.
 *
 * @param g a pointer to a grammar structure
 * @return PRS_OK if no error occurs, otherwise a different error code
 */
prs_ErrCode prs_resolveSymbols(struct fg_Grammar *g);

/**
 * Reads a raw grammar from a stream
 *
 * This function requires a pointer to a buffer that
 * will be allocated depending on the size of the input.
 * The buffer should be NULL before calling this function.
 *
 * The caller has the responsability of freeing this buffer.
 *
 * If an allocation error occurs, then -1 will be returned.
 *
 * @param stream input stream
 * @param pBuffer pointer to a NULL buffer
 * @return length of the extracted content
 */
ssize_t prs_readGrammar(FILE *stream, char **pBuffer);

/**
 * Separates items from any delimiter in the given string.
 *
 * Example : RULE= -> will be separated into 2 items : RULE, =
 * Separated items (ex: =) will be added into the list with
 * the given iterator. They will be placed between the original item
 * and the next one. The list order will be preserved.
 *
 * The delimiters string must be null terminated.
 *
 * @param it iterator on the item list
 * @param delimiters string of delimiters
 * @return number of added elements into the list
 */
int prs_splitDelimiters(struct ll_Iterator *it, const char *delimiters);

/**
 * Computes a set of potential first terminals for the given production rule item.
 *
 * We use dynamic programming to improve computation speed : it is usefull when there are
 * many references to other rules / tokens. The table parameter will be used to store intermediate
 * computations.
 * An entry in the table is represented by a pair (production rule / set of prs_ParserItem elements).
 *
 * A production rule can be optional if it is quantified by a star (ex: rule1 -> rule2*). In this case,
 * we should call this function with the following production rule item. This is the role of {@link prs_prFirst}.
 * An optional bool pointer can be passed to get this information.
 *
 * @param table a pointer to a hash table
 * @param pr a pointer to a production rule (list of fg_PrItem)
 * @param prItem a pointer to the first prItem of the production rule
 * @param pIsOptional pointer to a boolean that will be set to true if the prItem is optional
 * @return a pointer to a set of prs_ParserItem elements
 */
set_HashSet *prs_first(ht_Table *table, struct ll_LinkedList *pr, struct fg_PRItem *prItem, bool *pIsOptional);

/**
 * Computes a set of potential firsts terminals for the given production rule.
 *
 * This function handles the case of optional production rule items.
 *
 * @see prs_first for more explaination
 * @param table a pointer to a hash table of (ll_LinkedList / set of prs_ParserItem elements)
 * @param pr a pointer to a production rule
 * @return a pointer to a set of prs_ParserItem elements
 */
set_HashSet *prs_prFirst(ht_Table *table, struct ll_LinkedList *pr);

/**
 * Frees allocated memory for the given parser item.
 *
 * The given pointer will not be freed.
 *
 * @param parserItem a pointer to a parser item
 */
void prs_freeParserItem(prs_ParserItem *parserItem);

#endif // PARSER_H
