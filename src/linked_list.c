#include "linked_list.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

ll_LinkedList ll_createLinkedList() {
    ll_LinkedList list;
    memset(&list, 0, sizeof(list));

    return list;
}

void ll_freeLinkedList(ll_LinkedList *list, DataDestructor *destructor) {
    if (!list) {
        return;
    }

    ll_LinkedListItem *current = list->front;

    while (current) {
        ll_LinkedListItem *next = current->next;

        if (destructor) {
            destructor(current->data);
        }

        free(current);
        current = next;
    }

    list->front = list->back = NULL;
    list->size = 0;
}

void ll_forEachItem(ll_LinkedList *list, DataHandler *itemCallback, void *params) {
    assert(list);

    ll_LinkedListItem *current = list->front;

    while (current) {
        itemCallback(current->data, params);
        current = current->next;
    }
}

bool ll_isEqual(ll_LinkedList *l1, ll_LinkedList *l2, DataComparator *comparator) {
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

ll_Iterator ll_createIterator(ll_LinkedList *list) {
    assert(list);

    return (ll_Iterator){ .list = list, .current = list->front, .pEntry = &list->front };
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
