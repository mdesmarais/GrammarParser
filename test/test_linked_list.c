#include "unit.h"

#include <linked_list.h>
#include <log.h>

#include <stdbool.h>
#include <stdlib.h>

struct TestListIterator {
    int *elements[2];
    int pos;
};

static void testItemHandler(void *data, void *params) {
    int value = *((int*) data);
    struct TestListIterator *iterator = (struct TestListIterator*) params;

    int expected = *iterator->elements[iterator->pos++];

    ASSERT_INT_EQUALS("Checking list items", expected, value);
}

static void test_forEachItem() {
    int n1 = 89;
    int n2 = 3;

    ll_LinkedList list = ll_createLinkedList();
    ll_pushBackBatch(&list, 2, &n1, &n2);

    struct TestListIterator iterator;
    iterator.elements[0] = &n1;
    iterator.elements[1] = &n2;
    iterator.pos = 0;

    ll_forEachItem(&list, testItemHandler, &iterator);

    ll_freeLinkedList(&list, NULL);
}

static void test_pushBack() {
    int n1 = 10;
    int n2 = 69;
    ll_LinkedList list = ll_createLinkedList();

    ll_pushBack(&list, &n1);
    ll_pushBack(&list, &n2);

    ASSERT_INT_EQUALS("After an insertion, the size of the list should have been updated", 2, list.size);

    ASSERT_FALSE("The first element should not be null", list.front == NULL);
    ASSERT_FALSE("The last element should not be null", list.back == NULL);

    ll_LinkedListItem *item1 = list.front;
    ll_LinkedListItem *item2 = list.back;

    ASSERT_FALSE("First and last element should be different", item1 == item2);

    ASSERT_PTR_EQUALS("The first element's value should be 10", &n1, item1->data);

    ASSERT_PTR_EQUALS("Next element should be item2 (69)", item1->next, item2);

    ASSERT_PTR_EQUALS("The second and last element's value should be 64", &n2, item2->data);
    ASSERT_TRUE("As item2 is the last element, the next pointer should be null", item2->next == NULL);

    ll_freeLinkedList(&list, NULL);
}

static void test_pushBackBatch() {
    int n1 = 12;
    int n2 = 96;
    int n3 = 64;

    ll_LinkedList list = ll_createLinkedList();

    ll_pushBack(&list, &n1);
    ASSERT_INT_EQUALS("The list should have one element before inserting batch elements", 1, list.size);

    ll_pushBackBatch(&list, 2, &n2, &n3);
    ASSERT_INT_EQUALS("2 elements should have been inserted, the size of the list should be 3", 3, list.size);

    ll_LinkedListItem *item1 = list.front;
    ll_LinkedListItem *item2 = item1->next;
    ll_LinkedListItem *item3 = item2->next;

    ASSERT_PTR_EQUALS("The first item should be 12", &n1, item1->data);
    ASSERT_PTR_EQUALS("The second item should be 96", &n2, item2->data);
    ASSERT_PTR_EQUALS("The third and last item should be 64", &n3, item3->data);

    ll_freeLinkedList(&list, NULL);
}

static void test_iteratorInsert() {
    int n1 = 12;
    int n2 = 96;
    int n3 = 64;

    int n22 = 1;

    ll_LinkedList list = ll_createLinkedList();
    ll_pushBackBatch(&list, 3, &n1, &n2, &n3);

    ll_Iterator it = ll_createIterator(&list);
    ll_iteratorNext(&it);

    char *item2 = ll_iteratorNext(&it);
    ASSERT_INT_EQUALS("The second item should be 96", 96, *((int*) item2));

    ll_iteratorInsert(&it, &n22);

    ASSERT_INT_EQUALS("The size of the list should be 4 (instead of 3)", 4, list.size);

    char *item3 = ll_iteratorNext(&it);
    ASSERT_INT_EQUALS("The third item should be 1", 1, *((int*) item3));

    char *item4 = ll_iteratorNext(&it);
    ASSERT_INT_EQUALS("The last item should be 64", 64, *((int*) item4));

    ll_freeLinkedList(&list, NULL);
}

int main() {
    test_forEachItem();
    test_pushBack();
    test_pushBackBatch();
    test_iteratorInsert();
    return 0;
}
