#include <catch2/catch.hpp>

#include "helpers.hpp"

extern "C" {
#include <formal_grammar.h>
#include <linked_list.h>
}

using Catch::Matchers::Equals;

SCENARIO("A token can be extracted from a list of items", "[formal_grammar]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, (ll_DataDestructor*) prs_freeStringItem);
    fg_Token token = {};

    GIVEN("A token without a value") {
        fillItemList(&itemList, { "TOKEN", "=", ";" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, (const char *) nextStringItem(&it));
            REQUIRE(FG_TOKEN_MISSING_VALUE == res);
        }
    }

    GIVEN("A token without the ending semicolon") {
        fillItemList(&itemList, { "TOKEN", "=", "FUNC" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, (const char *) nextStringItem(&it));
            REQUIRE(FG_TOKEN_MISSING_END == res);
        }
    }

    GIVEN("A token without the equal sign (token name directly followed by its value") {
        fillItemList(&itemList, { "TOKEN", "`FUNC`", ";" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, (const char *) nextStringItem(&it));
            REQUIRE(FG_TOKEN_INVALID == res);
        }
    }

    GIVEN("A token without more items than its name") {
        fillItemList(&itemList, { "TOKEN" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, (const char *) nextStringItem(&it));
            REQUIRE(FG_TOKEN_INVALID == res);
        }
    }

    GIVEN("A valid string token with a quantifier") {
        fillItemList(&itemList, { "TOKEN", "=", "`FUNC`", "+", ";" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return ok and the token structure should have been updated") {
            int res = fg_extractToken(&token, &it, (const char *) nextStringItem(&it));
            REQUIRE(PRS_OK == res);

            REQUIRE(FG_STRING_TOKEN == token.type);
            REQUIRE_THAT("TOKEN", Equals(token.name));
            REQUIRE(PRS_PLUS_QUANTIFIER == token.quantifier);
            REQUIRE_THAT("FUNC", Equals(token.value.string));
        }
    }

    GIVEN("A valid range token with 2 ranges") {
        fillItemList(&itemList, { "TOKEN", "=", "[a-z2-4]", ";" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return OK") {
            int res = fg_extractToken(&token, &it, (const char *) nextStringItem(&it));
            REQUIRE(PRS_OK == res);

            AND_THEN("ranges pointer in token structure should have been updated") {
                struct fg_RangesToken *rangesToken = &token.value.rangesToken;
                REQUIRE(2 == rangesToken->rangesNumber);
                REQUIRE(rangesToken->ranges);
            }
        }
    }

    GIVEN("A token with a self reference") {
        fillItemList(&itemList, { "TOKEN", "=", "TOKEN", ";" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, (const char *) nextStringItem(&it));
            REQUIRE(FG_TOKEN_SELF_REF == res);
        }
    }

    GIVEN("A token with a ref on another token") {
        fillItemList(&itemList, { "TOKEN", "=", "TOKEN2", "?", ";" });
        ll_Iterator it = ll_createIterator(&itemList);

        int res = fg_extractToken(&token, &it, (const char *) nextStringItem(&it));

        THEN("It should return ok") {
            REQUIRE(PRS_OK == res);
        }

        AND_THEN("The token type should be a ref") {
            REQUIRE(FG_REF_TOKEN == token.type);
        }

        AND_THEN("The ref symbol should be on TOKEN2") {
            REQUIRE_THAT("TOKEN2", Equals(token.value.refToken.symbol));
        }
    }

    fg_freeToken(&token);
    ll_freeLinkedList(&itemList, NULL);
}

SCENARIO("A PRItem is either a reference to token or a rule", "[formal_grammar]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, (ll_DataDestructor*) free);

    fg_PRItem item = {};
    memset(&item, 0, sizeof(item));

    GIVEN("An unknown item type") {
        int res = fg_extractPrItem(&item, std::string("@token").c_str());

        THEN("It should return an error") {
            REQUIRE(FG_PRITEM_UNKNOWN_TYPE == res);
        }
    }

    GIVEN("A rule name") {
        std::string rule = "rule1";
        int res = fg_extractPrItem(&item, rule.c_str());

        THEN("It should return OK") {
            REQUIRE(PRS_OK == res);
        }

        AND_THEN("The item's type should be a rule") {
            REQUIRE(FG_RULE_ITEM == item.type);
        }

        AND_THEN("The reference symbol should be rule1") {
            REQUIRE_THAT("rule1", Equals(item.symbol));
        }
    }

    GIVEN("A token name") {
        std::string token = "TOKEN1";
        int res = fg_extractPrItem(&item, token.c_str());

        THEN("It should return OK") {
            REQUIRE(PRS_OK == res);
        }

        AND_THEN("The item's type should be a token") {
            REQUIRE(FG_TOKEN_ITEM == item.type);
        }

        AND_THEN("The reference symbol should be TOKEN1") {
            REQUIRE_THAT(token, Equals(item.symbol));
        }
    }

    ll_freeLinkedList(&itemList, NULL);
    fg_freePrItem(&item);
}

static void prItemDestructor(void *data, void *args) {
    fg_PRItem *prItem = (fg_PRItem*) data;
    fg_freePrItem(prItem);
    free(prItem);
}

SCENARIO("A production rule is made by one or more items (token or rule)", "[formal_grammar]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, (ll_DataDestructor*) prs_freeStringItem);

    ll_LinkedList productionRule;
    ll_createLinkedList(&productionRule, prItemDestructor);

    GIVEN("An iterator on a valid production rule items") {
        fillItemList(&itemList, { "rule1", "TOKEN1", "|" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return OK") {
            char *lastItem = NULL;
            int res = fg_extractProductionRule(&productionRule, &it, (char *) nextStringItem(&it), &lastItem);

            REQUIRE(PRS_OK == res);

            AND_THEN("The production rule should contain 2 items") {
                REQUIRE(2 == productionRule.size);
            }

            AND_THEN("The last item should be a pipe") {
                REQUIRE(lastItem);
                REQUIRE('|' == *lastItem);
            }
        }
    }

    GIVEN("An iterator on non valid production rule items") {
        fillItemList(&itemList, { "rule1", "()()", "|" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            char *lastItem = NULL;
            int res = fg_extractProductionRule(&productionRule, &it, (char *) nextStringItem(&it), &lastItem);

            REQUIRE(FG_PRITEM_UNKNOWN_TYPE == res);

            AND_THEN("The production rule should not have any items") {
                REQUIRE(0 == productionRule.size);
            }

            AND_THEN("The last character pointer should remain NULL") {
                REQUIRE_FALSE(lastItem);
            }
        }
    }

    GIVEN("An iterator on an empty production rule") {
        fillItemList(&itemList, { "|" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            char *lastItem = NULL;
            int res = fg_extractProductionRule(&productionRule, &it, (char *) nextStringItem(&it), &lastItem);

            REQUIRE(FG_PR_EMPTY == res);

            AND_THEN("The production rule should not have any items") {
                REQUIRE(0 == productionRule.size);
            }
        }
    }

    ll_freeLinkedList(&itemList, NULL);
    ll_freeLinkedList(&productionRule, NULL);
}

SCENARIO("A rule is made by one or more rules separated by a pipe and ends with a semicolon", "[formal_grammar]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, (ll_DataDestructor*) prs_freeStringItem);

    fg_Rule rule;
    fg_createRule(&rule);

    GIVEN("An iterator on a rule without the end marker") {
        fillItemList(&itemList, { "basic_rule", "=", "rule1" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractRule(&rule, &it, (char *) nextStringItem(&it));
            REQUIRE(FG_RULE_MISSING_END == res);
        }
    }

    GIVEN("An iterator on an empty rule") {
        fillItemList(&itemList, { "basic_rule", "=", ";" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractRule(&rule, &it, (char *) nextStringItem(&it));
            REQUIRE(FG_RULE_EMPTY == res);
        }
    }

    GIVEN("An iterator on a two production rules") {
        fillItemList(&itemList, { "basic_rule", "=", "rule1", "|", "TOKEN1", ";" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return OK") {
            int res = fg_extractRule(&rule, &it, (char *) nextStringItem(&it));
            REQUIRE(PRS_OK == res);
        }
    }

    ll_freeLinkedList(&itemList, NULL);
    fg_freeRule(&rule);
}
