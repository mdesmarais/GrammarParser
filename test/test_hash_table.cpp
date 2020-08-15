#include <catch2/catch.hpp>

extern "C" {
#include <hash_table.h>
#include <linked_list.h>
}

SCENARIO("A hash table is created with an initial capacity", "[hash_table]") {
    ht_Table table = {};

    GIVEN("A capacity of 5") {
        bool res = ht_createTable(&table, 5, ht_hashString, NULL);

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

    ht_freeTable(&table, NULL);
}

static uint32_t constantHash(const void *data) {
    data;
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
        ht_createTable(&table, 10, constantHash, intKeyComparator);

        int k1 = 1;
        int v1 = 38;

        ht_KVPair p1 = {};
        ht_createPair(&p1, &k1, &v1);

        ht_insertPair(&table, &p1);

        WHEN("Adding another element with same hash value") {
            int k2 = 2;
            int v2 = 87;

            ht_KVPair p2 = {};
            ht_createPair(&p2, &k2, &v2);
            ht_insertPair(&table, &p2);

            THEN("The size of the table should have been updated") {
                REQUIRE(2 == table.size);
            }

            AND_THEN("The two pairs should be in the first bucket") {
                ll_LinkedList *bucket = table.buckets;
                REQUIRE(2 == bucket->size);
                REQUIRE(ll_findItem(bucket, &p1, NULL));
                REQUIRE(ll_findItem(bucket, &p2, NULL));
            }
        }

        WHEN("Adding element with an existing key") {
            int k2 = 1;
            int v2 = 94;

            size_t previousSize = table.size;

            ht_KVPair p2 = {};
            ht_createPair(&p2, &k2, &v2);
            ht_insertPair(&table, &p2);

            THEN("The size of the table should not have been updated") {
                REQUIRE(previousSize == table.size);
            }

            AND_THEN("The first pair (p1)'s value should have been updated") {
                REQUIRE(&v2 == p1.value);
            }
        }

        ht_freeTable(&table, NULL);
    }
}

SCENARIO("A value can be retrieved by its key", "[hash_table]") {
    GIVEN("An hash table with 3 pairs") {
        ht_Table table = {};
        ht_createTable(&table, 10, intHash, intKeyComparator);

        int k1 = 1;
        int v1 = 84;
        int k2 = 2;
        int v2 = 64;
        int k3 = 3;
        int v3 = 78;

        ht_KVPair p1, p2, p3;
        ht_createPair(&p1, &k1, &v1);
        ht_createPair(&p2, &k2, &v2);
        ht_createPair(&p3, &k3, &v3);

        ht_insertPair(&table, &p1);
        ht_insertPair(&table, &p2);
        ht_insertPair(&table, &p3);

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

        ht_freeTable(&table, NULL);
    }
}

SCENARIO("A pair can be removed by its key", "[hash_table]") {
    GIVEN("An hash table with one pair") {
        ht_Table table = {};
        ht_createTable(&table, 10, intHash, intKeyComparator);

        int k1 = 1;
        int v1 = 45;

        ht_KVPair p1;
        ht_createPair(&p1, &k1, &v1);
        ht_insertPair(&table, &p1);

        WHEN("Removing k1") {
            ht_removeElement(&table, &k1, NULL);
            THEN("The table should be empty") {
                REQUIRE(0 == table.size);
            }
        }

        ht_freeTable(&table, NULL);
    }
}
