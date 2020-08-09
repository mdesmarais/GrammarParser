#include <catch2/catch.hpp>

extern "C" {
#include <formal_grammar.h>
#include <linked_list.h>
};

using Catch::Matchers::Equals;

SCENARIO("A token can be extracted from a list of items", "[formal_grammar]") {
    ll_LinkedList itemList = ll_createLinkedList();
    fg_Token token = {};

    GIVEN("A token without a value") {
        ll_pushBackBatch(&itemList, 3, "TOKEN", "=", ";");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, (const char*) ll_iteratorNext(&it));
            REQUIRE(FG_TOKEN_MISSING_VALUE == res);
        }
    }

    GIVEN("A token without the ending semicolon") {
        ll_pushBackBatch(&itemList, 3, "TOKEN", "=", "`FUNC`");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, (const char*) ll_iteratorNext(&it));
            REQUIRE(FG_TOKEN_MISSING_END == res);
        }
    }

    GIVEN("A token without the equal sign (token name directly followed by its value") {
        ll_pushBackBatch(&itemList, 3, "TOKEN", "`FUNC`", ";");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, (const char*) ll_iteratorNext(&it));
            REQUIRE(FG_TOKEN_INVALID == res);
        }
    }

    GIVEN("A token without more items than its name") {
        ll_pushBackBatch(&itemList, 1, "TOKEN");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, (const char*) ll_iteratorNext(&it));
            REQUIRE(FG_TOKEN_INVALID == res);
        }
    }

    GIVEN("A valid string token") {
        ll_pushBackBatch(&itemList, 4, "TOKEN", "=", "`FUNC`", ";");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return ok and the token structure should have been updated") {
            int res = fg_extractToken(&token, &it, (const char*) ll_iteratorNext(&it));
            REQUIRE(FG_OK == res);

            REQUIRE(FG_STRING_TOKEN == token.type);
            REQUIRE_THAT("TOKEN", Equals(token.name));
            REQUIRE_THAT("FUNC", Equals(token.string));
        }
    }

    fg_freeToken(&token);
    ll_freeLinkedList(&itemList, NULL);
}

