#include <catch2/catch.hpp>

extern "C" {
#include <linked_list.h>
}

SCENARIO("Items can be added at the end of the list individually", "[linked_list]") {
    GIVEN("An empty list") {
        ll_LinkedList list = ll_createLinkedList();

        WHEN("Adding two integers") {
            int n1 = 10;
            int n2 = 69;

            ll_pushBack(&list, &n1);
            ll_pushBack(&list, &n2);

            ll_LinkedListItem *item1 = list.front;
            ll_LinkedListItem *item2 = list.back;

            THEN("The size of the list should have been updated") {
                REQUIRE(2 == list.size);
            }

            AND_THEN("front and back items must not be null and different") {
                REQUIRE(list.front);
                REQUIRE(list.back);

                REQUIRE_FALSE(item1 == item2);
            }

            AND_THEN("Firt item's value should be n1") {
                REQUIRE(n1 == *((int*) item1->data));
                REQUIRE(item2 == item1->next);
            }

            AND_THEN("Second item's value should be n2") {
                REQUIRE(n2 == *((int*) item2->data));
                REQUIRE_FALSE(item2->next);
            }
        }

        ll_freeLinkedList(&list, NULL);
    }
}

SCENARIO("Several items can be added at the end of the list in a single func call", "[linked_list]") {
    ll_LinkedList itemList = ll_createLinkedList();

    GIVEN("A list with one item") {
        int n1 = 12;
        ll_pushBack(&itemList, &n1);

        WHEN("Adding a batch of 2 items") {
            int n2 = 96;
            int n3 = 87;

            ll_pushBackBatch(&itemList, 2, &n2, &n3);

            THEN("The list's should have been updated") {
                REQUIRE(3 == itemList.size);
            }

            AND_THEN("Thos 3 items should be at the end of the list") {
                ll_LinkedListItem *item1 = itemList.front;
                ll_LinkedListItem *item2 = item1->next;
                ll_LinkedListItem *item3 = item2->next;

                REQUIRE(n1 == *static_cast<int*>(item1->data));
                REQUIRE(n2 == *static_cast<int*>(item2->data));
                REQUIRE(n3 == *static_cast<int*>(item3->data));
            }
        }
    }

    ll_freeLinkedList(&itemList, NULL);
}

SCENARIO("A function can be applied to each item in the list", "[linked_list]") {
    ll_LinkedList list = ll_createLinkedList();
    GIVEN("A list with 2 integers") {
        int n1 = 89;
        int n2 = 3;

        ll_pushBackBatch(&list, 2, &n1, &n2);

        // 0 represents the end of the expected array
        int expected[] = { 89, 3, 0 };
        int *ptr = expected;

        THEN("The lambda function should be called with correct values") {
            ll_forEachItem(&list, [](void *data, void *params) {
                int value = *((int*) data);
                int **pPos = static_cast<int **>(params);

                // Prevents overflow
                // This function should only be called times
                REQUIRE_FALSE(0 == **pPos);

                REQUIRE(**pPos == value);

                *pPos += 1;
            }, &ptr);

            AND_THEN("This function should have been only called two times") {
                // We should be at the end delimiter after the call the foreach loop
                REQUIRE(0 == *ptr);
            }
        }
    }
    ll_freeLinkedList(&list, NULL);
}

static int intCmp(const void *n1, const void *n2) {
    const int *v1 = static_cast<const int *>(n1);
    const int *v2 = static_cast<const int *>(n2);

    return *v1 - *v2;
}

SCENARIO("Linked lists can be compared (equality)", "[linked_list]") {
    ll_LinkedList l1 = ll_createLinkedList();
    ll_LinkedList l2 = ll_createLinkedList();

    GIVEN("Two empty lists") {
        THEN("It should return true") {
            REQUIRE(ll_isEqual(&l1, &l2, intCmp));
        }
    }

    GIVEN("Two lists with different sizes but same n first items") {
        int n1 = 1;
        int n2 = 2;
        int n3 = 3;

        ll_pushBackBatch(&l1, 2, &n1, &n2);
        ll_pushBackBatch(&l2, 3, &n1, &n2, &n3);

        THEN("It should return false") {
            REQUIRE_FALSE(ll_isEqual(&l1, &l2, intCmp));
        }
    }

    GIVEN("Two lists with same size but with different items") {
        int n1 = 1;
        int n2 = 2;
        int n3 = 3;
        int n4 = 4;

        ll_pushBackBatch(&l1, 2, &n1, &n2);
        ll_pushBackBatch(&l2, 2, &n3, &n4);

        THEN("It should return false") {
            REQUIRE_FALSE(ll_isEqual(&l1, &l2, intCmp));
        }
    }

    GIVEN("Two lists with same items") {
        int n1 = 1;
        int n2 = 2;

        ll_pushBackBatch(&l1, 2, &n1, &n2);
        ll_pushBackBatch(&l2, 2, &n1, &n2);

        THEN("It should return true") {
            REQUIRE(ll_isEqual(&l1, &l2, intCmp));
        }
    }

    ll_freeLinkedList(&l1, NULL);
    ll_freeLinkedList(&l2, NULL);
}

SCENARIO("Items in list can be retrieved individually with an iterator", "[linked_list]") {
    ll_LinkedList itemList = ll_createLinkedList();

    GIVEN("A list with 3 items") {
        int n1 = 1;
        int n2 = 55;
        int n3 = 97;

        ll_pushBackBatch(&itemList, 3, &n1, &n2, &n3);

        ll_Iterator it = ll_createIterator(&itemList);

        THEN("Iterator next function should be callable 3 times") {
            const int *v1 = static_cast<const int *>(ll_iteratorNext(&it));
            REQUIRE(n1 == *v1);

            const int *v2 = static_cast<const int *>(ll_iteratorNext(&it));
            REQUIRE(n2 == *v2);

            const int *v3 = static_cast<const int *>(ll_iteratorNext(&it));
            REQUIRE(n3 == *v3);

            AND_THEN("Iterator should be at the end") {
                REQUIRE_FALSE(ll_iteratorHasNext(&it));
            }
        }
    }

    ll_freeLinkedList(&itemList, NULL);
}

SCENARIO("Items can be inserted into a list with an iterator", "[linked_list]") {
    ll_LinkedList itemList = ll_createLinkedList();

    GIVEN("A list with 3 items") {
        int n1 = 1;
        int n2 = 55;
        int n3 = 97;

        int n4 = 45;

        ll_pushBackBatch(&itemList, 3, &n1, &n2, &n3);

        ll_Iterator it = ll_createIterator(&itemList);

        WHEN("Moving iterator after the first item and insert a new item") {
            ll_iteratorNext(&it);
            ll_iteratorInsert(&it, &n4);

            THEN("The inserted item should put between 1 and 55") {
                ll_LinkedList expected = ll_createLinkedList();

                ll_pushBackBatch(&expected, 4, &n1, &n4, &n2, &n3);
                REQUIRE(ll_isEqual(&expected, &itemList, intCmp));

                ll_freeLinkedList(&expected, NULL);
            }
        }
    }

    ll_freeLinkedList(&itemList, NULL);
}
