#include <catch2/catch.hpp>

extern "C" {
#include <linked_list.h>
}

SCENARIO("Items can be added at the end of the list individually", "[linked_list]") {
    GIVEN("An empty list") {
        ll_LinkedList list;
        ll_createLinkedList(&list, NULL);

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
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, NULL);

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
    ll_LinkedList list;
    ll_createLinkedList(&list, NULL);
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
                int **pPos = (int**) params;

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
    ll_LinkedList l1, l2;
    ll_createLinkedList(&l1, NULL);
    ll_createLinkedList(&l2, NULL);

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

SCENARIO("Existence of an item in the list can be done with a comparator", "[linked_list]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, NULL);

    GIVEN("A list with several integers") {
        int n1 = 78;
        int n2 = 96;
        int n3 = 2;

        ll_pushBackBatch(&itemList, 3, &n1, &n2, &n3);

        WHEN("Looking for an unknown value") {
            int query = 1;
            void *result = ll_findItem(&itemList, &query, intCmp);

            THEN("It should return a null pointer") {
                REQUIRE_FALSE(result);
            }
        }

        WHEN("Looking for an existing value") {
            int query = 96;
            void *result = ll_findItem(&itemList, &query, intCmp);

            THEN("It should return a pointer on n2") {
                REQUIRE(&n2 == result);
            }
        }
    }

    ll_freeLinkedList(&itemList, NULL);
}

SCENARIO("An item can be removed from the list", "[linked_list]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, NULL);

    GIVEN("A list with three items") {
        int n1 = 1;
        int n2 = 55;
        int n3 = 787;

        ll_pushBackBatch(&itemList, 3, &n1, &n2, &n3);

        WHEN("Removing an unknown element") {
            int n3 = 78;
            bool result = ll_removeItem(&itemList, &n3, intCmp, NULL);

            THEN("It should return false") {
                REQUIRE_FALSE(result);
            }

            AND_THEN("The size of the list should not have changed") {
                REQUIRE(3 == itemList.size);
            }
        }

        WHEN("Deleting first element (n1)") {
            bool result = ll_removeItem(&itemList, &n1, intCmp, NULL);

            THEN("It should return true") {
                REQUIRE(result);
            }

            AND_THEN("The size of the list should have decreased") {
                REQUIRE(2 == itemList.size);
            }

            AND_THEN("The list's head should be n2") {
                REQUIRE(&n2 == itemList.front->data);
            }
        }

        WHEN("Deleting middle element (n2)") {
            ll_LinkedListItem *front = itemList.front;
            ll_LinkedListItem *back = itemList.back;

            bool result = ll_removeItem(&itemList, &n2, intCmp, NULL);

            THEN("It should return true") {
                REQUIRE(result);
            }

            AND_THEN("The list's size should have decreased") {
                REQUIRE(2 == itemList.size);
            }

            AND_THEN("Front en back pointers should not have changed") {
                REQUIRE(front == itemList.front);
                REQUIRE(back == itemList.back);
            }
        }

        WHEN("Deleting last element") {
            bool result = ll_removeItem(&itemList, &n3, intCmp, NULL);

            THEN("It should return true") {
                REQUIRE(result);
            }

            AND_THEN("The list's size should have decreased") {
                REQUIRE(2 == itemList.size);
            }

            AND_THEN("The back pointer should be on item n2") {
                REQUIRE(itemList.back);
                REQUIRE(&n2 == itemList.back->data);
            }
        }
    }

    ll_freeLinkedList(&itemList, NULL);
}

SCENARIO("Items in list can be retrieved individually with an iterator", "[linked_list]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, NULL);

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
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, NULL);

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
                ll_LinkedList expected;
                ll_createLinkedList(&expected, NULL);

                ll_pushBackBatch(&expected, 4, &n1, &n4, &n2, &n3);
                REQUIRE(ll_isEqual(&expected, &itemList, intCmp));

                ll_freeLinkedList(&expected, NULL);
            }
        }
    }

    ll_freeLinkedList(&itemList, NULL);
}
