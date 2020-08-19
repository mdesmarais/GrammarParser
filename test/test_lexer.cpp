#include <catch2/catch.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <string>
#include <sstream>

extern "C" {
#include <formal_grammar.h>
#include <lexer.h>
#include <linked_list.h>
};

using Catch::Matchers::Equals;

static char* allocateCString(std::string str) {
    char *string = (char*) calloc(str.size() + 1, 1);
    return strcpy(string, str.c_str());
}

SCENARIO("Create a letter range from two given chars", "[lexer]") {
    lex_Range range = {};

    GIVEN("The first character that is not a valid letter") {
        int result = lex_createLetterRange(&range, '$', 'f', false);

        THEN("It should return an error") {
            REQUIRE(LEXER_INVALID_CHAR_RANGE == result);
        }
    }

    GIVEN("The second character that is not a valid letter") {
        int result = lex_createLetterRange(&range, 'f', '$', false);

        THEN("It should return an error") {
            REQUIRE(LEXER_INVALID_CHAR_RANGE == result);
        }
    }

    GIVEN("First character is after the second in the lexicographic order") {
        int result = lex_createLetterRange(&range, 'f', 'b', false);

        THEN("It should return an error") {
            REQUIRE(LEXER_INVALID_RANGE == result);
        }
    }

    GIVEN("Two valid characters") {
        int result = lex_createLetterRange(&range, 'x', 'z', false);

        THEN("It should return OK") {
            REQUIRE(LEXER_OK == result);

            AND_THEN("range struct should have its first pointer on char x") {
                REQUIRE('x' == *range.start);
            }

            AND_THEN("range struct should have its second pointer on char z") {
                REQUIRE('z' == *(range.end - 1));
            }
        }
    }
}

SCENARIO("A range is made by two characters (alpha or digit) and a dash", "[lexer]") {
    lex_Range range = {};
    GIVEN("A pattern without a dash") {
        std::string input = "abc";

        THEN("It should return an error") {
            REQUIRE(LEXER_INVALID_RANGE_PATTERN == lex_extractRange(&range, input.c_str()));
        }
    }

    GIVEN("A pattern with an alpha char and a digit char") {
        std::string input = "a-7";

        THEN("It should return an error") {
            REQUIRE(LEXER_INVALID_RANGE_PATTERN == lex_extractRange(&range, input.c_str()));
        }
    }

    GIVEN("A valid letter range pattern") {
        std::string input = "a-b";

        THEN("It should return OK") {
            REQUIRE(LEXER_OK == lex_extractRange(&range, input.c_str()));

            AND_THEN("Range structure should have its first pointer on char a") {
                REQUIRE('a' == *range.start);
            }

            AND_THEN("Range structure should have its second pointer on char b") {
                REQUIRE('b' == *(range.end - 1));
            }
        }


    }

    GIVEN("A valid uppercase letter range pattern") {
        std::string input = "A-B";

        THEN("It should return OK") {
            REQUIRE(LEXER_OK == lex_extractRange(&range, input.c_str()));

            AND_THEN("Range structure should have its first pointer on char a") {
                REQUIRE('a' == *range.start);
            }

            AND_THEN("Range structure should have its second pointer on char b") {
                REQUIRE('b' == *(range.end - 1));
            }

            AND_THEN("Uppercase boolean should be set to true") {
                REQUIRE(range.uppercaseLetter);
            }
        }
    }

    GIVEN("A valid digit range pattern") {
        std::string input = "1-5";

        THEN("It should return OK") {
            REQUIRE(LEXER_OK == lex_extractRange(&range, input.c_str()));

            AND_THEN("Range structure should have its first pointer on char 1") {
                REQUIRE('1' == *range.start);
            }

            AND_THEN("Range structure should have its second pointer on char 5") {
                REQUIRE('5' == *(range.end - 1));
            }
        }
    }
}

SCENARIO("A range block ([...]) can contain several ranges", "[lexer]") {
    lex_Range *ranges = NULL;

    GIVEN("An empty string") {
        std::string input = "";

        THEN("It should return 0") {
            REQUIRE(0 == lex_extractRanges(&ranges, input.c_str(), input.size()));

            AND_THEN("The input pointer should remaiun unchanged (NULL)") {
                REQUIRE_FALSE(ranges);
            }
        }
    }

    GIVEN("A string with 3 ranges") {
        std::string input = "a-zA-Z0-9";

        THEN("It shoud return 3") {
            REQUIRE(3 == lex_extractRanges(&ranges, input.c_str(), input.size()));

            AND_THEN("The input pointer should have been modified") {
                REQUIRE(ranges);
            }
        }
    }

    GIVEN("A string with 2 valid patterns and one invalid") {
        std::string input = "a-z@-d1-3";

        THEN("It should return -1") {
            REQUIRE(-1 == lex_extractRanges(&ranges, input.c_str(), input.size()));

            AND_THEN("The input pointer should remaiun unchanged (NULL)") {
                REQUIRE_FALSE(ranges);
            }
        }
    }

    GIVEN("A string with one pattern and an additional char") {
        std::string input = "a-zb";

        THEN("It should return -1") {
            REQUIRE(-1 == lex_extractRanges(&ranges, input.c_str(), input.size()));

            AND_THEN("The input pointer should remaiun unchanged (NULL)") {
                REQUIRE_FALSE(ranges);
            }
        }
    }

    free(ranges);
    ranges = NULL;
}

SCENARIO("Items can be extracted from a raw grammar", "[lexer]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, (ll_DataDestructor*) free);

    GIVEN("A string containing a rule with several production rules") {
        std::string input = "op=      NUMBER \n"
                            "\t| SUB NUMBER \n"
                            "\t| `hello world`;";

        int extractedItems = lex_extractGrammarItems(input.c_str(), input.size(), &itemList);

        THEN("9 items should have been extracted") {
            REQUIRE(9 == extractedItems);

            ll_LinkedList expected;
            ll_createLinkedList(&expected, NULL);

            ll_pushBackBatch(&expected, 9, "op", "=", "NUMBER", "|", "SUB", "NUMBER", "|", "`hello world`", ";");

            REQUIRE(ll_isEqual(&itemList, &expected, (ll_DataComparator*) strcmp));

            ll_freeLinkedList(&expected, NULL);
        }
    }

    ll_freeLinkedList(&itemList, NULL);
}

SCENARIO("Read a grammar file into a dynamic buffer", "[lexer]") {
    GIVEN("A sample file") {
        FILE *f = fopen("data/test_01.txt", "r");
        REQUIRE(f);

        GIVEN("A null buffer") {
            char *buffer = NULL;

            ssize_t length = lex_readGrammar(f, &buffer);

            THEN("The buffer should not be null anymore") {
                REQUIRE(buffer);
                REQUIRE(length > 0);
            }

            AND_THEN("The buffer should be null terminated") {
                REQUIRE('\0' == buffer[length]);
            }

            AND_THEN("The buffer's content should equal to the file's content") {
                std::ifstream is { "data/test_01.txt"};
                REQUIRE(is.is_open());

                std::stringstream expected;
                expected << is.rdbuf();

                REQUIRE_THAT(expected.str(), Equals(buffer));
            }

            free(buffer);
        }

        fclose(f);
        f = NULL;
    }
}

SCENARIO("Extract tokens and rules from a list of items", "[lexer]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, NULL);

    fg_Grammar g;
    fg_createGrammar(&g);

    GIVEN("A list with a valid token and an unknown item") {
        ll_pushBackBatch(&itemList, 6, "TOKEN1", "=", "[a-z]", ";", "`hello`", ";");

        int res = lex_parseGrammarItems(&g, &itemList);

        THEN("It should return an error") {
            REQUIRE(LEXER_UNKNOWN_ITEM == res);
        }
    }

    GIVEN("A list with 1 rule and 1 token") {
        ll_pushBackBatch(&itemList, 8, "TOKEN1", "=", "`hello`", ";", "rule1", "=", "TOKEN1", ";");

        int res = lex_parseGrammarItems(&g, &itemList);

        THEN("It should return OK") {
            REQUIRE(LEXER_OK == res);
        }
    }

    GIVEN("A list with an invalid token") {
        ll_pushBackBatch(&itemList, 3, "TOKEN1", "=", ";");

        int res = lex_parseGrammarItems(&g, &itemList);

        THEN("It should return an error") {
            REQUIRE_FALSE(LEXER_OK == res);
        }
    }

    GIVEN("A list with an invalid rule") {
        ll_pushBackBatch(&itemList, 3, "rule", "TOKEN", ";");

        int res = lex_parseGrammarItems(&g, &itemList);

        THEN("It should return an error") {
            REQUIRE_FALSE(LEXER_OK == res);
        }
    }

    fg_freeGrammar(&g);
    ll_freeLinkedList(&itemList, NULL);
}

SCENARIO("symbols resolution is used to allow recursive rules", "[lexer]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, NULL);

    fg_Grammar g;
    fg_createGrammar(&g);

    GIVEN("A token that uses an unknown token") {
        ll_pushBackBatch(&itemList, 4, "TOKEN1", "=", "TOKEN2", ";");

        int res = lex_parseGrammarItems(&g, &itemList);
        REQUIRE(res == LEXER_OK);

        WHEN("resolving symbols") {
            res = lex_resolveSymbols(&g);

            THEN("It should return an error") {
                REQUIRE(res == FG_UNKNOWN_TOKEN);
            }
        }
    }

    GIVEN("A rule that uses an unknown rule") {
        ll_pushBackBatch(&itemList, 4, "rule1", "=", "rule2", ";");

        int res = lex_parseGrammarItems(&g, &itemList);
        REQUIRE(res == LEXER_OK);

        WHEN("resolving symbols") {
            res = lex_resolveSymbols(&g);

            THEN("It should return an error") {
                REQUIRE(res == FG_UNKNOWN_RULE);
            }
        }
    }

    GIVEN("A rule that uses an unknown token") {
        ll_pushBackBatch(&itemList, 4, "rule1", "=", "TOKEN1", ";");

        int res = lex_parseGrammarItems(&g, &itemList);
        REQUIRE(res == LEXER_OK);

        WHEN("resolving symbols") {
            res = lex_resolveSymbols(&g);

            THEN("It should return an error") {
                REQUIRE(res == FG_UNKNOWN_TOKEN);
            }
        }
    }

    GIVEN("Two rules : one is using the other and a token") {
        ll_pushBackBatch(&itemList, 15, "rule1", "=", "rule1", "TOKEN1", "|", "rule2", ";",
                         "TOKEN1", "=", "[a-z]", ";", "rule2", "=", "TOKEN1", ";");

        int res = lex_parseGrammarItems(&g, &itemList);
        REQUIRE(res == LEXER_OK);

        WHEN("resolving symbols") {
            res = lex_resolveSymbols(&g);

            THEN("It should return OK") {
                REQUIRE(LEXER_OK == res);
            }

            AND_THEN("Resolution on rule1's production rule should have be done") {
                fg_Rule *rule1 = (fg_Rule*) ht_getValue(&g.rules, "rule1");

                ll_Iterator it = ll_createIterator((ll_LinkedList*) rule1->productionRuleList.front->data);

                // Testing recursive rule
                fg_PRItem *prItem1 = (fg_PRItem*) ll_iteratorNext(&it);
                REQUIRE(rule1 == prItem1->rule);

                fg_PRItem *prItem2 = (fg_PRItem*) ll_iteratorNext(&it);
                fg_Token *token1 = (fg_Token*) ht_getValue(&g.tokens, "TOKEN1");
                REQUIRE(token1 == prItem2->token);
            }
        }
    }

    GIVEN("Two tokens : one is referencing the other") {
        ll_pushBackBatch(&itemList, 8, "TOKEN1", "=", "TOKEN2", ";", "TOKEN2", "=", "`test`", ";");

        int res = lex_parseGrammarItems(&g, &itemList);
        REQUIRE(res == LEXER_OK);

        WHEN("Resolving symbols") {
            res = lex_resolveSymbols(&g);

            THEN("It should return ok") {
                REQUIRE(LEXER_OK == res);
            }

            AND_THEN("TOKEN1 should have a reference on TOKEN2") {
                fg_Token *token1 = (fg_Token*) ht_getValue(&g.tokens, "TOKEN1");
                fg_Token *token2 = (fg_Token*) ht_getValue(&g.tokens, "TOKEN2");

                REQUIRE(token1);
                REQUIRE(token2);

                REQUIRE(token2 == token1->value.refToken.token);
            }
        }
    }

    fg_freeGrammar(&g);
    ll_freeLinkedList(&itemList, NULL);
}
