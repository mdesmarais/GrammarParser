#include <catch2/catch.hpp>

#include "helpers.hpp"

extern "C" {
#include <parser.h>
#include <parser_errors.h>
#include <formal_grammar.h>
#include <collections/linked_list.h>
}

using Catch::Matchers::Equals;

SCENARIO("A token can be extracted from a list of items", "[formal_grammar]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, (ll_DataDestructor*) prs_freeStringItem);
    fg_Token token = {};

    GIVEN("A token without a value") {
        fillItemList(&itemList, { "%TOKEN", "=", ";" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, (prs_StringItem*) ll_iteratorNext(&it));
            REQUIRE(FG_TOKEN_MISSING_VALUE == res);
        }
    }

    GIVEN("A token without the ending semicolon") {
        fillItemList(&itemList, { "%TOKEN", "=", "FUNC" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, (prs_StringItem*) ll_iteratorNext(&it));
            REQUIRE(FG_TOKEN_MISSING_END == res);
        }
    }

    GIVEN("A token without the equal sign (token name directly followed by its value") {
        fillItemList(&itemList, { "%TOKEN", "`FUNC`", ";" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, (prs_StringItem*) ll_iteratorNext(&it));
            REQUIRE(FG_TOKEN_INVALID == res);
        }
    }

    GIVEN("A token without more items than its name") {
        fillItemList(&itemList, { "%TOKEN" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, (prs_StringItem*) ll_iteratorNext(&it));
            REQUIRE(FG_TOKEN_INVALID == res);
        }
    }

    GIVEN("A valid string token with a quantifier") {
        fillItemList(&itemList, { "%TOKEN", "=", "`FUNC`", "+", ";" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return ok and the token structure should have been updated") {
            int res = fg_extractToken(&token, &it, (prs_StringItem*) ll_iteratorNext(&it));
            REQUIRE(PRS_OK == res);

            REQUIRE(FG_STRING_TOKEN == token.type);
            REQUIRE_THAT("TOKEN", Equals(token.name));
            REQUIRE(PRS_PLUS_QUANTIFIER == token.quantifier);
            REQUIRE_THAT("FUNC", Equals(token.value.string));
        }
    }

    GIVEN("A valid range token with 2 ranges") {
        fillItemList(&itemList, { "%TOKEN", "=", "[a-z2-4]", ";" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return OK") {
            int res = fg_extractToken(&token, &it, (prs_StringItem*) ll_iteratorNext(&it));
            REQUIRE(PRS_OK == res);

            AND_THEN("ranges pointer in token structure should have been updated") {
                prs_RangeArray *rangeArray = &token.value.rangeArray;
                REQUIRE(2 == rangeArray->size);
                REQUIRE(rangeArray->ranges);
            }
        }
    }

    GIVEN("A token with a self reference") {
        fillItemList(&itemList, { "%TOKEN", "=", "TOKEN", ";" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, (prs_StringItem*) ll_iteratorNext(&it));
            REQUIRE(FG_TOKEN_SELF_REF == res);
        }
    }

    GIVEN("A token with a ref on another token") {
        fillItemList(&itemList, { "%TOKEN", "=", "TOKEN2", "?", ";" });
        ll_Iterator it = ll_createIterator(&itemList);

        int res = fg_extractToken(&token, &it, (prs_StringItem*) ll_iteratorNext(&it));

        THEN("It should return ok") {
            REQUIRE(PRS_OK == res);
        }

        AND_THEN("The token type should be a ref") {
            REQUIRE(FG_REF_TOKEN == token.type);
        }

        AND_THEN("The ref symbol should be on TOKEN2") {
            REQUIRE_THAT("TOKEN2", Equals(token.value.refToken.symbol->item));
        }
    }

    fg_freeToken(&token);
    ll_freeLinkedList(&itemList, nullptr);
}

SCENARIO("A production rule item has a type and a value (or a reference)", "[formal_grammar]") {
    fg_PRItem prItem;
    memset(&prItem, 0, sizeof(prItem));

    GIVEN("An unknown item type") {
        prs_StringItem stringItem = { .item = (char*) "@token", .line = 0, .column = 0 };
        int res = fg_extractPRItem(&prItem, &stringItem);

        THEN("It should return an error") {
            REQUIRE(FG_PRITEM_UNKNOWN_TYPE == res);
        }
    }

    GIVEN("A reference to a rule") {
        prs_StringItem stringItem = { .item = (char*) "rule1", .line = 0, .column = 0 };
        int res = fg_extractPRItem(&prItem, &stringItem);

        THEN("It should return ok") {
            REQUIRE(PRS_OK == res);
        }

        AND_THEN("prItem type and symbol fields should have been updated") {
            REQUIRE(FG_RULE_ITEM == prItem.type);
            REQUIRE_THAT("rule1", Equals(prItem.symbol->item));
        }
    }

    GIVEN("A string block without the end marker") {
        prs_StringItem stringItem = { .item = (char*) "`hello", .line = 0, .column = 0 };
        int res = fg_extractPRItem(&prItem, &stringItem);

        THEN("It should return an error") {
            REQUIRE(FG_STRING_BLOCK_MISSING_END == res);
        }
    }

    GIVEN("An empty string block (with start and end markers)") {
        prs_StringItem stringItem = { .item = (char*) "``", .line = 0, .column = 0 };
        int res = fg_extractPRItem(&prItem, &stringItem);

        THEN("It should return an error") {
            REQUIRE(FG_STRING_BLOCK_EMPTY == res);
        }
    }

    GIVEN("A valid string block") {
        prs_StringItem stringItem = { .item = (char*) "`hello`", .line = 0, .column = 0 };
        int res = fg_extractPRItem(&prItem, &stringItem);

        THEN("It should return ok") {
            REQUIRE(PRS_OK == res);
        }

        AND_THEN("prItem type and value fields should have been updated") {
            REQUIRE(FG_STRING_ITEM == prItem.type);
            REQUIRE_THAT("hello", Equals(prItem.value.string));
        }
    }

    fg_freePRItem(&prItem);
}

SCENARIO("A production rule is made by one or more items (token or rule)", "[formal_grammar]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, (ll_DataDestructor*) prs_freeStringItem);

    ll_LinkedList productionRule;
    ll_createLinkedList(&productionRule, (ll_DataDestructor*) free);

    GIVEN("An iterator on a valid production rule items") {
        fillItemList(&itemList, { "rule1", "TOKEN1", "|" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return OK") {
            prs_StringItem *lastItem = nullptr;
            int res = fg_extractProductionRule(&productionRule, &it, (prs_StringItem*) ll_iteratorNext(&it), &lastItem);

            REQUIRE(PRS_OK == res);

            AND_THEN("The production rule should contain 2 items") {
                REQUIRE(2 == productionRule.size);
            }

            AND_THEN("The last item should be a pipe") {
                REQUIRE(lastItem);
                REQUIRE('|' == *lastItem->item);
            }
        }
    }

    GIVEN("An iterator on non valid production rule items") {
        fillItemList(&itemList, { "rule1", "()()", "|" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            prs_StringItem *lastItem = nullptr;
            int res = fg_extractProductionRule(&productionRule, &it, (prs_StringItem*) ll_iteratorNext(&it), &lastItem);

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
            prs_StringItem *lastItem = nullptr;
            int res = fg_extractProductionRule(&productionRule, &it, (prs_StringItem*) ll_iteratorNext(&it), &lastItem);

            REQUIRE(FG_PR_EMPTY == res);

            AND_THEN("The production rule should not have any items") {
                REQUIRE(0 == productionRule.size);
            }
        }
    }

    ll_freeLinkedList(&itemList, nullptr);
    ll_freeLinkedList(&productionRule, nullptr);
}

SCENARIO("A rule is made by one or more rules separated by a pipe and ends with a semicolon", "[formal_grammar]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, (ll_DataDestructor*) prs_freeStringItem);

    fg_Rule rule;
    fg_createRule(&rule);

    GIVEN("An iterator on a rule without the end marker") {
        fillItemList(&itemList, { "%basic_rule", "=", "rule1" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractRule(&rule, &it, (prs_StringItem*) ll_iteratorNext(&it));
            REQUIRE(FG_RULE_MISSING_END == res);
        }
    }

    GIVEN("An iterator on an empty rule") {
        fillItemList(&itemList, { "%basic_rule", "=", ";" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractRule(&rule, &it, (prs_StringItem*) ll_iteratorNext(&it));
            REQUIRE(FG_RULE_EMPTY == res);
        }
    }

    GIVEN("An iterator on a two production rules") {
        fillItemList(&itemList, { "%basic_rule", "=", "rule1", "|", "TOKEN1", ";" });
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return OK") {
            int res = fg_extractRule(&rule, &it, (prs_StringItem*) ll_iteratorNext(&it));
            REQUIRE(PRS_OK == res);
        }
    }

    ll_freeLinkedList(&itemList, nullptr);
    fg_freeRule(&rule);
}
