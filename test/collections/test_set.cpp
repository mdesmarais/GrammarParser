#include <catch2/catch.hpp>

#include <cstring>

extern "C" {
#include <collections/hash_table.h>
#include <collections/set.h>
}

static int intCmp(const void *n1, const void *n2) {
    return ((int*) n1) - ((int*) n2);
}

SCENARIO("A set is based on hash tables", "[set]") {
    set_HashSet set;

    GIVEN("An empty set") {
        set_createSet(&set, 10, ht_hashString, (ht_KeyComparator *) strcmp, NULL, nullptr);

        WHEN("Adding a string element") {
            std::string e1 = "elem1";
            set_insertValue(&set, (void *) e1.c_str());

            THEN("The set's size should be 1") {
                REQUIRE(1 == set.size);
            }

            AND_WHEN("Checking if inserted element exists") {
                bool result = set_contains(&set, (void *) e1.c_str());

                THEN("It should return TRUE") {
                    REQUIRE(result);
                }
            }
        }

        WHEN("Checking if an unknown element exists") {
            bool result = set_contains(&set, (void*) "unknown");

            THEN("It should return false") {
                REQUIRE_FALSE(result);
            }
        }
    }

    set_freeSet(&set);
}

SCENARIO("Two sets can be merged with an union", "[set]") {
    set_HashSet s1;
    set_createSet(&s1, 10, ht_hashString, (ht_KeyComparator *) intCmp, nullptr, nullptr);

    set_HashSet s2;
    set_createSet(&s2, 10, ht_hashString, (ht_KeyComparator *) intCmp, nullptr, nullptr);

    GIVEN("Two empty sets") {
        WHEN("Doing union between s1 and s2") {
            set_union(&s1, &s2);

            THEN("s1 should be still empty after union") {
                REQUIRE(0 == s1.size);
            }
        }
    }

    GIVEN("Two equal sets") {
        int n = 12;

        set_insertValue(&s1, &n);
        set_insertValue(&s2, &n);

        WHEN("Doing union between s1 and s2") {
            set_union(&s1, &s2);

            THEN("s1 should not have been modified") {
                REQUIRE(1 == s1.size);
                REQUIRE(set_contains(&s1, &n));
            }
        }
    }

    GIVEN("Two different sets") {
        int n1 = 12;
        set_insertValue(&s1, &n1);

        int n2 = 45;
        set_insertValue(&s2, &n2);

        WHEN("Doing union between s1 and s2") {
            set_union(&s1, &s2);

            THEN("s1 should contain n1 and n2") {
                REQUIRE(2 == s1.size);
                REQUIRE(set_contains(&s1, &n1));
                REQUIRE(set_contains(&s1, &n2));
            }
        }
    }

    set_freeSet(&s1);
    set_freeSet(&s2);
}
