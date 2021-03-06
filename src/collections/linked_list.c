#include "linked_list.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void ll_createLinkedList(ll_LinkedList *list, ll_DataDestructor *destructor) {
    list->front = list->back = NULL;
    list->size = 0;
    list->destructor = destructor;
}

void ll_freeLinkedList(ll_LinkedList *list, void *args) {
    if (!list) {
        return;
    }

    ll_LinkedListItem *current = list->front;

    while (current) {
        ll_LinkedListItem *next = current->next;

        if (list->destructor) {
            list->destructor(current->data, args);
        }

        free(current);
        current = next;
    }

    list->front = list->back = NULL;
    list->size = 0;
}

void ll_forEachItem(ll_LinkedList *list, ll_DataHandler *itemCallback, void *params) {
    assert(list);

    ll_LinkedListItem *current = list->front;

    while (current) {
        itemCallback(current->data, params);
        current = current->next;
    }
}

bool ll_isEqual(ll_LinkedList *l1, ll_LinkedList *l2, ll_DataComparator *comparator) {
    assert(l1);
    assert(l2);

    if (l1->size != l2->size) {
        return false;
    }

    ll_Iterator it1 = ll_createIterator(l1);
    ll_Iterator it2 = ll_createIterator(l2);

    while (ll_iteratorHasNext(&it1)) {
        const void *data1 = ll_iteratorNext(&it1);
        const void *data2 = ll_iteratorNext(&it2);

        if (comparator(data1, data2) != 0) {
            return false;
        }
    }

    return true;
}

void ll_pushBack(ll_LinkedList *list, void *data) {
    assert(list);
    assert(data);

    ll_LinkedListItem *item = malloc(sizeof(*item));

    if (!item) {
        return;
    }

    item->data = data;
    item->next = NULL;

    if (list->back) {
        list->back->next = item;
        list->back = item;
    }
    else {
        list->front = list->back = item;
    }

    list->size += 1;
}

void ll_pushBackBatch(ll_LinkedList *list, int itemsNumber, ...) {
    assert(list);
    assert(itemsNumber > 0);

    va_list itemList;

    va_start(itemList, itemsNumber);

    for (int i = 0;i < itemsNumber;++i) {
        void *item = va_arg(itemList, void*);
        ll_pushBack(list, item);
    }

    va_end(itemList);
}

void *ll_findItem(ll_LinkedList *list, const void *query, ll_DataComparator *comparator) {
    assert(list);
    assert(query);

    ll_Iterator it = ll_createIterator(list);

    while (ll_iteratorHasNext(&it)) {
        void *value = ll_iteratorNext(&it);

        if ((comparator && comparator(query, value) == 0) || (!comparator && query == value)) {
            return value;
        }
    }

    return NULL;
}

bool ll_removeItem(ll_LinkedList *list, const void *item, ll_DataComparator *comparator, void *args) {
    assert(list);
    assert(item);

    ll_LinkedListItem *previousItem = NULL;
    ll_LinkedListItem *currentItem = list->front;


    while (currentItem) {
        if ((comparator && comparator(item, currentItem->data) == 0) || (!comparator && item == currentItem->data)) {
            if (previousItem) {
                previousItem->next = currentItem->next;
            }

            if (currentItem == list->front) {
                list->front = currentItem->next;
            }

            if (currentItem == list->back) {
                list->back = previousItem;
            }

            if (list->destructor) {
                list->destructor(currentItem->data, args);
            }
            free(currentItem);

            --list->size;

            return true;
        }

        previousItem = currentItem;
        currentItem = currentItem->next;
    }

    return false;
}

ll_Iterator ll_createIterator(ll_LinkedList *list) {
    assert(list);

    return (ll_Iterator){ .list = list, .current = list->front, .pEntry = &list->front };
}

void ll_initIterator(ll_Iterator *it, ll_LinkedList *list) {
    assert(it);
    assert(list);

    it->list = list;
    it->current = list->front;
    it->pEntry = &list->front;
}

bool ll_iteratorHasNext(ll_Iterator *it) {
    assert(it);

    return it->current != NULL;
}

void *ll_iteratorNext(ll_Iterator *it) {
    assert(it);
    assert(ll_iteratorHasNext(it));

    void *data = it->current->data;
    it->pEntry = &it->current->next;
    it->current = it->current->next;

    return data;
}

void ll_iteratorInsert(ll_Iterator *it, void *data) {
    assert(it);
    assert(data);

    ll_LinkedListItem *item = malloc(sizeof(*item));
    item->data = data;

    ll_LinkedListItem *next = *it->pEntry;
    *it->pEntry = item;
    item->next = next;

    it->current = item;
    it->pEntry = &item->next;

    ++it->list->size;
}
