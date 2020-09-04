#include <catch2/catch.hpp>

extern "C" {
#include <hash_table.h>
#include <linked_list.h>
}

SCENARIO("A hash table is created with an initial capacity", "[hash_table]") {
    ht_Table table = {};

    GIVEN("A capacity of 5") {
        bool res = ht_createTable(&table, 5, ht_hashString, nullptr, nullptr);

        THEN("It should return true") {
            REQUIRE(res);
        }

        AND_THEN("All 5 buckets should be empty") {
            REQUIRE(5 == table.capacity);
            REQUIRE(table.buckets);

            for (size_t i = 0;i < 5;++i) {
                REQUIRE(0 == table.buckets[i].size);
            }
        }

        AND_THEN("The table should be empty") {
            REQUIRE(0 == table.size);
        }
    }

    ht_freeTable(&table);
}

static uint32_t constantHash() {
    return 0;
}

static uint32_t intHash(const void *data) {
    return *((int*) data);
}

static int intKeyComparator(const void *d1, const void *d2) {
    return *((int*) d1) - *((int*) d2);
}

SCENARIO("Pair insertion with hash collision", "[hash_table]") {
    GIVEN("An hash table with one element and a constant hash function") {
        ht_Table table = {};
        ht_createTable(&table, 10, (ht_HashFunction*) constantHash, intKeyComparator, nullptr);

        int k1 = 1;
        int v1 = 38;

        ht_insertElement(&table, &k1, &v1);

        WHEN("Adding another element with same hash value") {
            int k2 = 2;
            int v2 = 87;

            ht_insertElement(&table, &k2, &v2);

            THEN("The size of the table should have been updated") {
                REQUIRE(2 == table.size);
            }

            AND_THEN("The two pairs should be in the first bucket") {
                ll_LinkedList *bucket = table.buckets;
                REQUIRE(2 == bucket->size);
            }
        }

        WHEN("Adding element with an existing key") {
            int k2 = 1;
            int v2 = 94;

            size_t previousSize = table.size;

            ht_insertElement(&table, &k2, &v2);

            THEN("The size of the table should not have been updated") {
                REQUIRE(previousSize == table.size);
            }

            AND_THEN("The value should have been updated") {
                int *result = (int*) ht_getValue(&table, &k2);
                REQUIRE(result);
                REQUIRE(&v2 == result);
            }
        }

        ht_freeTable(&table);
    }
}

SCENARIO("A value can be retrieved by its key", "[hash_table]") {
    GIVEN("An hash table with 3 pairs") {
        ht_Table table = {};
        ht_createTable(&table, 10, intHash, intKeyComparator, nullptr);

        int k1 = 1;
        int v1 = 84;
        int k2 = 2;
        int v2 = 64;
        int k3 = 3;
        int v3 = 78;

        ht_insertElement(&table, &k1, &v1);
        ht_insertElement(&table, &k2, &v2);
        ht_insertElement(&table, &k3, &v3);

        WHEN("Looking for an unknown key") {
            int k = 7;
            void *result = ht_getValue(&table, &k);

            THEN("It should return a null pointer") {
                REQUIRE_FALSE(result);
            }
        }

        WHEN("Looking for an existing key") {
            void *result = ht_getValue(&table, &k2);

            THEN("It should return a pointer on v2") {
                REQUIRE(&v2 == result);
            }
        }

        WHEN("Gathering values into an array") {
            int **values = reinterpret_cast<int **>(ht_getValues(&table));

            THEN("It should contain v1, v2 and v3") {
                REQUIRE(&v1 == *values);
                REQUIRE(&v2 == values[1]);
                REQUIRE(&v3 == values[2]);
            }

            free(values);
        }

        ht_freeTable(&table);
    }
}

SCENARIO("A pair can be removed by its key", "[hash_table]") {
    GIVEN("An hash table with one pair") {
        ht_Table table = {};
        ht_createTable(&table, 10, intHash, intKeyComparator, nullptr);

        int k1 = 1;
        int v1 = 45;

        ht_insertElement(&table, &k1, &v1);

        WHEN("Removing k1") {
            ht_removeElement(&table, &k1);
            THEN("The table should be empty") {
                REQUIRE(0 == table.size);
            }
        }

        ht_freeTable(&table);
    }
}

SCENARIO("Items in table can be retrieved individually with an iterator", "[hash_table]") {
    ht_Table table;
    ht_createTable(&table, 6, intHash, intKeyComparator, nullptr);

    GIVEN("An empty map") {
        ht_Iterator it;
        ht_createIterator(&it, &table);

        THEN("Iterator should be at the end") {
            REQUIRE_FALSE(ht_iteratorHasNext(&it));
        }
    }

    GIVEN("A map with 3 pairs") {
        int n1 = 1;
        int v1 = 78;

        int n2 = 2;
        int v2 = 98;

        int n3 = 4;
        int v3 = 74;

        ht_insertElement(&table, &n1, &v1);
        ht_insertElement(&table, &n2, &v2);
        ht_insertElement(&table, &n3, &v3);

        // We are using a simple hash function that allows us to control
        // which bucket will be used to store a pair.

        // With those values, we have : bucket0=empty, bucket1=pair1,
        // bucket2=pair2, bucket3=empty, bucket4=pair3, bucket5=empty

        ht_Iterator it;
        ht_createIterator(&it, &table);

        THEN("Iterator next function should be callable 3 times") {
            ht_KVPair *p1 = ht_iteratorNext(&it);
            REQUIRE(&n1 == (int*) p1->key);
            REQUIRE(&v1 == (int*) p1->value);

            ht_KVPair *p2 = ht_iteratorNext(&it);
            REQUIRE(&n2 == (int*) p2->key);
            REQUIRE(&v2 == (int*) p2->value);

            ht_KVPair *p3 = ht_iteratorNext(&it);
            REQUIRE(&n3 == (int*) p3->key);
            REQUIRE(&v3 == (int*) p3->value);

            AND_THEN("Iterator should be at the end") {
                REQUIRE_FALSE(ht_iteratorHasNext(&it));
            }
        }
    }

    ht_freeTable(&table);
}
