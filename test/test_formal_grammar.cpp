#include <catch2/catch.hpp>

extern "C" {
#include <formal_grammar.h>
#include <linked_list.h>
#include <set.h>
}

using Catch::Matchers::Equals;

static char *createCString(std::string str) {
    char *cstr = (char*) calloc(str.size() + 1, 1);
    return strcpy(cstr, str.c_str());
}

static void createToken(ht_Table *tokensMap, std::string name) {
    fg_Token *t = (fg_Token*) malloc(sizeof(*t));
    memset(t, 0, sizeof(*t));
    t->name = createCString(name);

    ht_insertElement(tokensMap, t->name, t);
}

static void createRule(ht_Table *rulesMap, std::string name) {
    fg_Rule *r = (fg_Rule*) malloc(sizeof(*r));
    memset(r, 0, sizeof(*r));
    r->name = createCString(name);

    ht_insertElement(rulesMap, r->name, r);
}

static fg_Grammar createBasicGrammar() {
    fg_Grammar g = {};
    fg_createGrammar(&g);

    createToken(&g.tokens, "TOKEN1");
    createToken(&g.tokens, "TOKEN2");
    createToken(&g.tokens, "TOKEN3");

    createRule(&g.rules, "rule1");
    createRule(&g.rules, "rule2");
    createRule(&g.rules, "rule3");

    return g;
}

SCENARIO("A token can be extracted from a list of items", "[formal_grammar]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, NULL);
    fg_Token token = {};

    set_HashSet symbols;
    set_createSet(&symbols, 10, (ht_KeyComparator*) strcmp, (set_ElementDestructor*) free);

    GIVEN("A token without a value") {
        ll_pushBackBatch(&itemList, 3, "TOKEN", "=", ";");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, &symbols, (const char*) ll_iteratorNext(&it));
            REQUIRE(FG_TOKEN_MISSING_VALUE == res);
        }
    }

    GIVEN("A token without the ending semicolon") {
        ll_pushBackBatch(&itemList, 3, "TOKEN", "=", "`FUNC`");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, &symbols, (const char*) ll_iteratorNext(&it));
            REQUIRE(FG_TOKEN_MISSING_END == res);
        }
    }

    GIVEN("A token without the equal sign (token name directly followed by its value") {
        ll_pushBackBatch(&itemList, 3, "TOKEN", "`FUNC`", ";");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, &symbols, (const char*) ll_iteratorNext(&it));
            REQUIRE(FG_TOKEN_INVALID == res);
        }
    }

    GIVEN("A token without more items than its name") {
        ll_pushBackBatch(&itemList, 1, "TOKEN");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, &symbols, (const char*) ll_iteratorNext(&it));
            REQUIRE(FG_TOKEN_INVALID == res);
        }
    }

    GIVEN("A valid string token with a quantifier") {
        ll_pushBackBatch(&itemList, 5, "TOKEN", "=", "`FUNC`", "+", ";");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return ok and the token structure should have been updated") {
            int res = fg_extractToken(&token, &it, &symbols, (const char*) ll_iteratorNext(&it));
            REQUIRE(FG_OK == res);

            REQUIRE(FG_STRING_TOKEN == token.type);
            REQUIRE_THAT("TOKEN", Equals(token.name));
            REQUIRE(LEX_PLUS_QUANTIFIER == token.quantifier);
            REQUIRE_THAT("FUNC", Equals(token.value.string));
        }
    }

    GIVEN("A valid range token with 2 ranges") {
        ll_pushBackBatch(&itemList, 4, "TOKEN", "=", "[a-z2-4]", ";");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return OK") {
            int res = fg_extractToken(&token, &it, &symbols, (const char*) ll_iteratorNext(&it));
            REQUIRE(FG_OK == res);

            AND_THEN("ranges pointer in token structure should have been updated") {
                struct fg_RangesToken *rangesToken = &token.value.ranges;
                REQUIRE(2 == rangesToken->rangesNumber);
                REQUIRE(rangesToken->ranges);
            }
        }
    }

    GIVEN("A token with a self reference") {
        ll_pushBackBatch(&itemList, 4, "TOKEN", "=", "TOKEN", ";");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractToken(&token, &it, &symbols, (const char*) ll_iteratorNext(&it));
            REQUIRE(FG_TOKEN_SELF_REF == res);
        }
    }

    GIVEN("One existing symbol (another token)") {
        char *tokenSymbol = createCString("TOKEN2");
        set_insertValue(&symbols, tokenSymbol);

        GIVEN("A token with a ref on this token") {
            ll_pushBackBatch(&itemList, 5, "TOKEN", "=", "TOKEN2", "?", ";");
            ll_Iterator it = ll_createIterator(&itemList);

            int res = fg_extractToken(&token, &it, &symbols, (const char*) ll_iteratorNext(&it));

            THEN("It should return ok") {
                REQUIRE(FG_OK == res);
            }

            AND_THEN("The token type should be a ref") {
                REQUIRE(FG_REF_TOKEN == token.type);
            }

            AND_THEN("The ref should be on TOKEN2") {
                REQUIRE_THAT("TOKEN2", Equals(token.value.refToken));
            }
        }
    }

    fg_freeToken(&token);
    ll_freeLinkedList(&itemList, NULL);
    set_freeSet(&symbols);
}

SCENARIO("A PRItem is either a reference to an existing token or a rule", "[formal_grammar]") {
    fg_Grammar g = createBasicGrammar();
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, NULL);

    fg_PRItem item = {};
    memset(&item, 0, sizeof(item));

    GIVEN("An unknown token name") {
        int res = fg_extractPrItem(&item, std::string("Token8").c_str(), &g);
        THEN("It should return an error") {
            REQUIRE(FG_UNKNOWN_TOKEN == res);
        }
    }

    GIVEN("An unknown rule name") {
        int res = fg_extractPrItem(&item, std::string("rule13").c_str(), &g);
        THEN("It should return an error") {
            REQUIRE(FG_UNKNOWN_RULE == res);
        }
    }

    GIVEN("An unknown item type") {
        int res = fg_extractPrItem(&item, std::string("@token").c_str(), &g);

        THEN("It should return an error") {
            REQUIRE(FG_PRITEM_UNKNOWN_TYPE == res);
        }
    }

    GIVEN("An existing rule name") {
        std::string rule = "rule1";
        int res = fg_extractPrItem(&item, rule.c_str(), &g);

        THEN("It should return OK") {
            REQUIRE(FG_OK == res);
        }

        AND_THEN("The item's type should be a rule") {
            REQUIRE(FG_RULE_ITEM == item.type);
        }

        AND_THEN("The field rule should point on the first grammar's rule") {
            fg_Rule *expected = (fg_Rule*) ht_getValue(&g.rules, rule.c_str());
            REQUIRE(expected);
            REQUIRE(expected == item.rule);
        }
    }

    GIVEN("An existing token name") {
        std::string token = "TOKEN1";
        int res = fg_extractPrItem(&item, token.c_str(), &g);

        THEN("It should return OK") {
            REQUIRE(FG_OK == res);
        }

        AND_THEN("The item's type should be a token") {
            REQUIRE(FG_TOKEN_ITEM == item.type);
        }

        AND_THEN("The field token should poin on the first grammar's token") {
            fg_Token *expected = (fg_Token*) ht_getValue(&g.tokens, token.c_str());
            REQUIRE(expected);
            REQUIRE(expected == item.token);
        }
    }

    ll_freeLinkedList(&itemList, NULL);
    fg_freeGrammar(&g);
}

SCENARIO("A production rule is made by one or more items (token or rule)", "[formal_grammar]") {
    fg_Grammar g = createBasicGrammar();
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, NULL);

    ll_LinkedList productionRule;
    ll_createLinkedList(&productionRule, (ll_DataDestructor*) free);

    GIVEN("An iterator on a valid production rule items") {
        ll_pushBackBatch(&itemList, 3, "rule1", "TOKEN1", "|");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return OK") {
            char *lastItem = NULL;
            int res = fg_extractProductionRule(&productionRule, &it, &g, (char*) ll_iteratorNext(&it), &lastItem);

            REQUIRE(FG_OK == res);

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
        ll_pushBackBatch(&itemList, 3, "rule1", "()()", "|");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            char *lastItem = NULL;
            int res = fg_extractProductionRule(&productionRule, &it, &g, (char*) ll_iteratorNext(&it), &lastItem);

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
        ll_pushBackBatch(&itemList, 1, "|");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            char *lastItem = NULL;
            int res = fg_extractProductionRule(&productionRule, &it, &g, (char*) ll_iteratorNext(&it), &lastItem);

            REQUIRE(FG_PR_EMPTY == res);

            AND_THEN("The production rule should not have any items") {
                REQUIRE(0 == productionRule.size);
            }
        }
    }

    ll_freeLinkedList(&itemList, NULL);
    ll_freeLinkedList(&productionRule, NULL);
    fg_freeGrammar(&g);
}

SCENARIO("A rule is made by one or more rules separated by a pipe and ends with a semicolon", "[formal_grammar]") {
    fg_Grammar g = createBasicGrammar();
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, NULL);

    fg_Rule rule;
    fg_createRule(&rule);

    GIVEN("An iterator on a rule without the end marker") {
        ll_pushBackBatch(&itemList, 3, "basic_rule", "=", "rule1");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractRule(&rule, &it, &g, (char*) ll_iteratorNext(&it));
            REQUIRE(FG_RULE_MISSING_END == res);
        }
    }

    GIVEN("An iterator on an empty rule") {
        ll_pushBackBatch(&itemList, 3, "basic_rule", "=", ";");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return an error") {
            int res = fg_extractRule(&rule, &it, &g, (char*) ll_iteratorNext(&it));
            REQUIRE(FG_RULE_EMPTY == res);
        }
    }

    GIVEN("An iterator on a two production rules") {
        ll_pushBackBatch(&itemList, 6, "basic_rule", "=", "rule1", "|", "TOKEN1", ";");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return OK") {
            int res = fg_extractRule(&rule, &it, &g, (char*) ll_iteratorNext(&it));
            REQUIRE(FG_OK == res);
        }
    }

    ll_freeLinkedList(&itemList, NULL);
    fg_freeGrammar(&g);
    fg_freeRule(&rule);
}
