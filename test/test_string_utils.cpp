#include <catch2/catch.hpp>

#include <stdlib.h>

extern "C" {
#include <linked_list.h>
#include <string_utils.h>
};

using Catch::Matchers::Equals;

SCENARIO("Gets pointer of a delimiter from a list in a given string", "[string_utils]") {
    std::string delimiters = "+-";

    GIVEN("A string without any of given delimiters") {
        std::string input = "hello world";

        THEN("It should return a null pointer") {
            REQUIRE_FALSE(str_findDelimiter(input.c_str(), input.size(), delimiters.c_str(), delimiters.size()));
        }
    }

    GIVEN("A string with 2 of given delimiters") {
        std::string input = "hello+worl-d";

        THEN("it should return pointer on the '+' char") {
            const char *expected = input.c_str() + 5;
            char *res = str_findDelimiter(input.c_str(), input.size(), delimiters.c_str(), delimiters.size());
            REQUIRE(expected == res);
        }
    }
}

SCENARIO("Removes all whitespaces from a given string", "[string_utils]") {
    GIVEN("A string full of whitespaces") {
        std::string input = "     ";

        THEN("It should return 0") {
            char target[20] = {};
            REQUIRE(0 == str_removeWhitespaces(target, input.c_str(), input.size()));

            AND_THEN("The target string should remain empty") {
                REQUIRE(0 == strlen(target));
            }
        }
    }

    GIVEN("A string with 3 whitespaces") {
        std::string input = " hello world ";

        THEN("It should return 10") {
            char target[20] = {};
            REQUIRE(10 == str_removeWhitespaces(target, input.c_str(), input.size()));

            AND_THEN("The target string should be equal to 'helloworld'") {
                REQUIRE_THAT("helloworld", Equals(std::string(target)));
            }
        }
    }
}

SCENARIO("Consecutive spaces are removed", "[string_utils") {
    GIVEN("A string with 3 consecutive spaces") {
        std::string input = " \n\t";

        THEN("It should return 1") {
            char target[10] = {};

            REQUIRE(1 == str_removeMultipleSpaces(target, input.c_str(), input.size()));

            AND_THEN("The target string should be equal to ' '") {
                REQUIRE_THAT(" ", Equals(std::string(target)));
            }
        }
    }

    GIVEN("A string with pre and suffix equal to 2 spaces") {
        std::string input = "  hello world  ";

        THEN("it should have removed 2 spaces") {
            char target[20] = {};

            REQUIRE(13 == str_removeMultipleSpaces(target, input.c_str(), input.size()));

            AND_THEN("The target string should be equal to ' hello world '") {
                REQUIRE_THAT(" hello world ", Equals(std::string(target)));
            }
        }
    }
}

SCENARIO("String can be splitted with by a delimiter", "[string_utils]") {
    GIVEN("A string with 3 items and several occurences of the delimiter") {
        std::string input = ",hello,world,,,  ";

        THEN("It should have extracted 3 items") {
            ll_LinkedList itemList;
            ll_createLinkedList(&itemList, (ll_DataDestructor*) free);

            REQUIRE(3 == str_splitItems(input.c_str(), input.size(), &itemList, ','));

            ll_LinkedList expected;
            ll_createLinkedList(&expected, NULL);
            ll_pushBackBatch(&expected, 3, "hello", "world", "  ");

            REQUIRE(ll_isEqual(&expected, &itemList, reinterpret_cast<int (*)(const void *, const void *)>(strcmp)));

            ll_freeLinkedList(&expected, NULL);
            ll_freeLinkedList(&itemList, NULL);
        }
    }
}
