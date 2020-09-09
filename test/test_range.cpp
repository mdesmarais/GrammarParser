#include <catch2/catch.hpp>

#include <cstring>

extern "C" {
#include <range.h>
}

SCENARIO("Create a letter range from two given chars", "[range]") {
    prs_Range range = {};

    GIVEN("The first character that is not a valid letter") {
        int result = prs_createLetterRange(&range, '$', 'f', false);

        THEN("It should return an error") {
            REQUIRE(PRS_INVALID_CHAR_RANGE == result);
        }
    }

    GIVEN("The second character that is not a valid letter") {
        int result = prs_createLetterRange(&range, 'f', '{', false);

        THEN("It should return an error") {
            REQUIRE(PRS_INVALID_CHAR_RANGE == result);
        }
    }

    GIVEN("First character is after the second in the lexicographic order") {
        int result = prs_createLetterRange(&range, 'f', 'b', false);

        THEN("It should return an error") {
            REQUIRE(PRS_INVALID_RANGE == result);
        }
    }

    GIVEN("Two valid characters") {
        int result = prs_createLetterRange(&range, 'x', 'z', false);

        THEN("It should return OK") {
            REQUIRE(PRS_OK == result);

            AND_THEN("range struct should have its first pointer on char x") {
                REQUIRE('x' == range.start);
            }

            AND_THEN("range struct should have its second pointer on char z") {
                REQUIRE('z' == range.end - 1);
            }
        }
    }
}

SCENARIO("A range is made by two characters (alpha or digit) and a dash", "[range]") {
    prs_Range range = {};
    GIVEN("A pattern without a dash") {
        std::string input = "abc";

        THEN("It should return an error") {
            REQUIRE(PRS_INVALID_RANGE_PATTERN == prs_extractRange(&range, input.c_str()));
        }
    }

    GIVEN("A pattern with an alpha char and a digit char") {
        std::string input = "a-7";

        THEN("It should return an error") {
            REQUIRE(PRS_INVALID_RANGE_PATTERN == prs_extractRange(&range, input.c_str()));
        }
    }

    GIVEN("A valid letter range pattern") {
        std::string input = "a-b";

        THEN("It should return OK") {
            REQUIRE(PRS_OK == prs_extractRange(&range, input.c_str()));

            AND_THEN("Range structure should have its first pointer on char a") {
                REQUIRE('a' == range.start);
            }

            AND_THEN("Range structure should have its second pointer on char b") {
                REQUIRE('b' == range.end - 1);
            }
        }


    }

    GIVEN("A valid uppercase letter range pattern") {
        std::string input = "A-B";

        THEN("It should return OK") {
            REQUIRE(PRS_OK == prs_extractRange(&range, input.c_str()));

            AND_THEN("Range structure should have its first pointer on char a") {
                REQUIRE('a' == range.start);
            }

            AND_THEN("Range structure should have its second pointer on char b") {
                REQUIRE('b' == range.end - 1);
            }

            AND_THEN("Uppercase boolean should be set to true") {
                REQUIRE(range.uppercaseLetter);
            }
        }
    }

    GIVEN("A valid digit range pattern") {
        std::string input = "1-5";

        THEN("It should return OK") {
            REQUIRE(PRS_OK == prs_extractRange(&range, input.c_str()));

            AND_THEN("Range structure should have its first pointer on char 1") {
                REQUIRE('1' == range.start);
            }

            AND_THEN("Range structure should have its second pointer on char 5") {
                REQUIRE('5' == range.end - 1);
            }
        }
    }
}

SCENARIO("A range block ([...]) can contain several ranges", "[range]") {
    prs_RangeArray rangeArray;
    memset(&rangeArray, 0, sizeof(rangeArray));

    GIVEN("An empty string") {
        std::string input;
        prs_ErrCode res = prs_extractRanges(&rangeArray, input.c_str(), input.size());

        THEN("It should return ok") {
            REQUIRE(PRS_OK == res);
        }

        AND_THEN("The array should still be empty") {
            REQUIRE_FALSE(rangeArray.ranges);
            REQUIRE(0 == rangeArray.size);
        }
    }

    GIVEN("A string with 3 ranges") {
        std::string input = "a-zA-Z0-9";
        prs_ErrCode res = prs_extractRanges(&rangeArray, input.c_str(), input.size());

        THEN("It should return ok") {
            REQUIRE(PRS_OK == res);
        }

        AND_THEN("The array should contain 3 ranges") {
            REQUIRE(rangeArray.ranges);
            REQUIRE(3 == rangeArray.size);
        }
    }

    GIVEN("A string with 2 valid patterns and one invalid") {
        std::string input = "a-z@-d1-3";
        prs_ErrCode res = prs_extractRanges(&rangeArray, input.c_str(), input.size());

        THEN("It should return an error") {
            REQUIRE(PRS_INVALID_RANGE_PATTERN == res);
        }

        AND_THEN("The input pointer should remain unchanged (nullptr)") {
            REQUIRE_FALSE(rangeArray.ranges);
            REQUIRE(0 == rangeArray.size);
        }
    }

    GIVEN("A string with one pattern and an additional char") {
        std::string input = "a-zb";
        int res = prs_extractRanges(&rangeArray, input.c_str(), input.size());

        THEN("It should return an error") {
            REQUIRE(PRS_INVALID_RANGE_PATTERN == res);
        }

        AND_THEN("The input pointer should remain unchanged (nullptr)") {
            REQUIRE_FALSE(rangeArray.ranges);
            REQUIRE(0 == rangeArray.size);
        }
    }

    prs_freeRangeArray(&rangeArray);
}
