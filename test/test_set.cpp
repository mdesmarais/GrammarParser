#include <catch2/catch.hpp>

#include <string.h>

extern "C" {
#include <hash_table.h>
#include <set.h>
}

SCENARIO("A set is based on hash tables", "[hash_table][set]") {
    set_HashSet set;

    GIVEN("An empty set") {
        set_createSet(&set, 10, (ht_KeyComparator*) strcmp, NULL);

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

            THEN("It should return false");
        }
    }

    set_freeSet(&set);
}
