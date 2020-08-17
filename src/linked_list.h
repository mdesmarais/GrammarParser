#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

typedef int ll_DataComparator(const void*, const void*);
typedef void ll_DataDestructor(void*, void*);
typedef void ll_DataHandler(void*, void*);

typedef struct ll_LinkedListItem {
    void *data;
    struct ll_LinkedListItem *next;
} ll_LinkedListItem;

typedef struct ll_LinkedList {
    ll_LinkedListItem *front;
    ll_LinkedListItem *back;
    size_t size;
    ll_DataDestructor *destructor;
} ll_LinkedList;

typedef struct ll_Iterator {
    ll_LinkedList *list;
    ll_LinkedListItem *current;
    ll_LinkedListItem **pEntry;
} ll_Iterator;

/**
 * Creates an empty linked list
 *
 * @param list a pointer to a linked list
 * @param destructor pointer to a function destructor, can be null
 */
void ll_createLinkedList(ll_LinkedList *list, ll_DataDestructor *destructor);

/**
 * Frees allocated memory and empty a linked list.
 *
 * Additional args for the destructor function can be a NULL pointer.
 *
 * @param list pointer to a linked list
 * @param args additional args that will be passed to the destructor
 */
void ll_freeLinkedList(ll_LinkedList *list, void *args);

/**
 * Calls a function for each element in the given list.
 *
 * Additional parameters can be passed to the function with
 * argument 'params'.
 *
 * @param list a pointer to a linked list
 * @param itemCallback pointer to a function that will receive each element
 * @param params additional params to the function, can be NULL
 */
void ll_forEachItem(ll_LinkedList *list, ll_DataHandler *itemCallback, void *params);

/**
 * Checks if two lists are equal.
 *
 * If they does not have the same size then
 * false will be returned.
 *
 * Each elements are compared with the given comparator function.
 *
 * @param l1 a pointer to a list
 * @param l2 a pointer to a list
 * @param comparator pointer to a comparator function
 * @return true if the two given lists are equal, otherwise false
 */
bool ll_isEqual(ll_LinkedList *l1, ll_LinkedList *l2, ll_DataComparator *comparator);

/**
 * Inserts a new element at the end of the list.
 *
 * @param list a pointer to a linked list
 * @param data pointer to a data to insert
 */
void ll_pushBack(ll_LinkedList *list, void *data);

/**
 * Inserts several elements at the end of the list.
 *
 * @param list a pointer to a linked list
 * @param itemsNumber number of items to insert
 * @param ... itemsNumber pointers
 */
void ll_pushBackBatch(ll_LinkedList *list, int itemsNumber, ...);

/**
 * Finds an element in a given list using a comparator.
 *
 * @param list a pointer to a linked list
 * @param query value to be searched
 * @param comparator pointer to a comparator function
 * @return the value if it exists in the list, otherwise false
 */
void *ll_findItem(ll_LinkedList *list, const void *query, ll_DataComparator *comparator);

/**
 * Removes an element from a given list.
 *
 * Additional args for the destructor function can be a NULL pointer.
 *
 * If the element does not exist then the function
 * returns false.
 *
 * @param list a pointer to a linked list
 * @param item item to remove
 * @param comparator pointer to a comparator function
 * @param args additional args that will be passed to the destructor
 * @return true if the element has been removed, otherwise false
 */
bool ll_removeItem(ll_LinkedList *list, const void *item, ll_DataComparator *comparator, void *args);

/**
 * Creates an iterator set on the list's front.
 *
 * @param list a pointer to a linked list
 * @return an iterator
 */
ll_Iterator ll_createIterator(ll_LinkedList *list);

void ll_initIterator(ll_Iterator *it, ll_LinkedList *list);

/**
 * Checks if one or more elements are available on the list at
 * the iterator's position.
 *
 * @param it pointer to an iterator
 * @return true if there is one more element, otherwise false
 */
bool ll_iteratorHasNext(ll_Iterator *it);

/**
 * Moves the iterator to the next element and returns the current value.
 *
 * The iterator must not be at the end of the list, otherwise an assertion will
 * be triggered.
 *
 * @param it pointer to an iterator
 * @return current value
 */
void *ll_iteratorNext(ll_Iterator *it);

/**
 * Inserts a new element at the current position.
 *
 * A call to ll_iteratorNext after the insertion will return
 * the inserted element.
 *
 * @param it pointer to an iterator
 * @param data element to insert
 */
void ll_iteratorInsert(ll_Iterator *it, void *data);

#endif // LINKED_LIST_H
