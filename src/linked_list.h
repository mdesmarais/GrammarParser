#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct ll_LinkedListItem {
    void *data;
    struct ll_LinkedListItem *next;
} ll_LinkedListItem;

typedef struct ll_LinkedList {
    ll_LinkedListItem *front;
    ll_LinkedListItem *back;
    size_t size;
} ll_LinkedList;

typedef struct ll_Iterator {
    ll_LinkedList *list;
    ll_LinkedListItem *current;
    ll_LinkedListItem **pEntry;
} ll_Iterator;

typedef int DataComparator(const void*, const void*);
typedef void DataDestructor(void*);
typedef void DataHandler(void*, void*);

ll_LinkedList ll_createLinkedList();
void ll_freeLinkedList(ll_LinkedList *list, DataDestructor *destructor);

void ll_forEachItem(ll_LinkedList *list, DataHandler *itemCallback, void *params);
bool ll_isEqual(ll_LinkedList *l1, ll_LinkedList *l2, DataComparator *comparator);

void ll_pushBack(ll_LinkedList *list, void *data);
void ll_pushBackBatch(ll_LinkedList *list, int itemsNumber, ...);

void *ll_findItem(ll_LinkedList *list, void *query, DataComparator *comparator);

ll_Iterator ll_createIterator(ll_LinkedList *list);
bool ll_iteratorHasNext(ll_Iterator *it);
void *ll_iteratorNext(ll_Iterator *it);
void ll_iteratorInsert(ll_Iterator *it, void *data);

#endif // LINKED_LIST_H
