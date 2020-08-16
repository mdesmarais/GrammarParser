#include <catch2/catch.hpp>

#include <stdlib.h>
#include <string.h>

#include <string>

extern "C" {
#include <lexer.h>
#include <linked_list.h>
};

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