#include <catch2/catch.hpp>

#include <stdlib.h>
#include <string.h>

#include <string>

extern "C" {
#include <lexer.h>
#include <linked_list.h>
};

SCENARIO("", "extractGrammarItems") {
    ll_LinkedList itemList = ll_createLinkedList();
    GIVEN("A string containing a rule with several production rules") {
        std::string input = "op=      NUMBER \n"
                            "\t| SUB NUMBER \n"
                            "\t| `hello world`;";

        int extractedItems = lex_extractGrammarItems(input.c_str(), input.size(), &itemList);

        THEN("9 items should have been extracted") {
            REQUIRE(9 == extractedItems);

            ll_LinkedList expected = ll_createLinkedList();
            ll_pushBackBatch(&expected, 9, "op", "=", "NUMBER", "|", "SUB", "NUMBER", "|", "`hello world`", ";");

            REQUIRE(ll_isEqual(&itemList, &expected, reinterpret_cast<int (*)(const void *, const void *)>(&strcmp)));

            ll_freeLinkedList(&expected, NULL);
        }
    }

    ll_freeLinkedList(&itemList, free);
}