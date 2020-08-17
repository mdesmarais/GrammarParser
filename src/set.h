#ifndef SET_H
#define SET_H

#include "hash_table.h"

#include <stdbool.h>
#include <string.h>

typedef struct ht_Table set_HashSet;
typedef void set_ElementDestructor(void*);

void set_createSet(set_HashSet *set, size_t initialCapacity, ht_KeyComparator *comparator, set_ElementDestructor *destructor);
void set_freeSet(set_HashSet *set);

void set_insertValue(set_HashSet *set, void *value);
bool set_contains(set_HashSet *set, const void *value);

#endif // SET_H
