#include <catch2/catch.hpp>

extern "C" {
#include <formal_grammar.h>
#include <linked_list.h>
}

using Catch::Matchers::Equals;

/*static char *createCString(std::string str) {
    char *cstr = (char*) calloc(str.size() + 1, 1);
    return strcpy(cstr, str.c_str());
}

static fg_Token *createToken(std::string name) {
    fg_Token *t = (fg_Token*) malloc(sizeof(*t));
    memset(t, 0, sizeof(*t));
    t->name = createCString(name);
    return t;
}

static fg_Rule *createRule(std::string name) {
    fg_Rule *r = (fg_Rule*) malloc(sizeof(*r));
    memset(r, 0, sizeof(*r));
    r->name = createCString(name);
    return r;
}

static fg_Grammar createBasicGrammar() {
    fg_Token *t1 = createToken("TOKEN1");
    fg_Token *t2 = createToken("TOKEN2");
    fg_Token *t3 = createToken("TOKEN3");

    fg_Rule *r1 = createRule("rule1");
    fg_Rule *r2 = createRule("rule2");
    fg_Rule *r3 = createRule("rule3");

    fg_Grammar g = {};
    fg_createGrammar(&g);
    g.ruleList = ll_createLinkedList();
    g.tokenList = ll_createLinkedList();

    ll_pushBackBatch(&g.tokenList, 3, t1, t2, t3);
    ll_pushBackBatch(&g.ruleList, 3, r1, r2, r3);

    return g;
}

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

    GIVEN("A valid range token with 2 ranges") {
        ll_pushBackBatch(&itemList, 4, "TOKEN", "=", "[a-z2-4]", ";");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return OK") {
            int res = fg_extractToken(&token, &it, (const char*) ll_iteratorNext(&it));
            REQUIRE(FG_OK == res);

            AND_THEN("ranges pointer in token structure should have been updated") {
                REQUIRE(2 == token.rangesNumber);
                REQUIRE(token.ranges);
            }
        }
    }

    GIVEN("A valid range token with 2 ranges and a quantifier") {
        ll_pushBackBatch(&itemList, 5, "TOKEN", "=", "[a-z2-4]", "+", ";");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return OK") {
            int res = fg_extractToken(&token, &it, (const char*) ll_iteratorNext(&it));
            REQUIRE(FG_OK == res);

            AND_THEN("ranges pointer in token structure should have been updated") {
                REQUIRE(2 == token.rangesNumber);
                REQUIRE(token.ranges);
            }

            AND_THEN("each range should have its quantifier field updated") {
                REQUIRE(LEX_PLUS_QUANTIFIER == token.ranges[0].quantifier);
                REQUIRE(LEX_PLUS_QUANTIFIER == token.ranges[1].quantifier);
            }
        }
    }

    fg_freeToken(&token);
    ll_freeLinkedList(&itemList, NULL);
}

SCENARIO("A token can be retrieved from a list by its name", "[formal_grammar]") {
    GIVEN("Basic grammar with several tokens") {
        fg_Grammar g = createBasicGrammar();

        WHEN("Looking for a token that does not exist") {
            ll_Iterator tokenIt = ll_createIterator(&g.tokenList);
            fg_Token *token = fg_getTokenByName("UNKNOWN", &tokenIt);

            THEN("It should return a null pointer") {
                REQUIRE_FALSE(token);
            }
        }

        WHEN("Looking for an existing token") {
            ll_Iterator tokenIt = ll_createIterator(&g.tokenList);
            fg_Token *token = fg_getTokenByName("TOKEN2", &tokenIt);

            THEN("It should return a pointer to the second rule in the list") {
                REQUIRE(token == g.tokenList.front->next->data);
            }
        }

        fg_freeGrammar(&g);
    }
}

SCENARIO("A rule can be retrieved from a list by its name", "[formal_grammar]") {
    GIVEN("Basic grammar with several rules") {
        fg_Grammar g = createBasicGrammar();

        WHEN("Looking for a rule that does not exist") {
            ll_Iterator ruleIt = ll_createIterator(&g.ruleList);
            fg_Rule *rule = fg_getRuleByName("unknown", &ruleIt);

            THEN("It should return a null pointer") {
                REQUIRE_FALSE(rule);
            }
        }

        WHEN("Looking for an existing rule") {
            ll_Iterator ruleIt = ll_createIterator(&g.ruleList);
            fg_Rule *rule = fg_getRuleByName("rule2", &ruleIt);

            THEN("It should return a pointer to the second rule in the list") {

                REQUIRE(rule == g.ruleList.front->next->data);
            }
        }

        fg_freeGrammar(&g);
    }
}

SCENARIO("A PRItem is either a reference to an existing token or a rule", "[formal_grammar]") {
    fg_Grammar g = createBasicGrammar();
    ll_LinkedList itemList = ll_createLinkedList();

    ll_Iterator tokenIt = ll_createIterator(&g.tokenList);
    ll_Iterator ruleIt = ll_createIterator(&g.ruleList);

    fg_PRItem item = {};
    memset(&item, 0, sizeof(item));

    GIVEN("An unknown token name") {
        THEN("It should return an error") {
            int res = fg_extractPrItem(&item, std::string("Token8").c_str(), &tokenIt, &ruleIt);
            REQUIRE(FG_UNKNOWN_TOKEN == res);
        }
    }

    GIVEN("An unknown rule name") {
        THEN("It should return an error") {
            int res = fg_extractPrItem(&item, std::string("rule13").c_str(), &tokenIt, &ruleIt);
            REQUIRE(FG_UNKNOWN_RULE == res);
        }
    }

    GIVEN("An unknown item type") {
        THEN("It should return an error") {
            int res = fg_extractPrItem(&item, std::string("@token").c_str(), &tokenIt, &ruleIt);
            REQUIRE(FG_PRITEM_UNKNOWN_TYPE == res);
        }
    }

    GIVEN("An existing token name") {
        THEN("It should return OK") {
            int res = fg_extractPrItem(&item, std::string("rule1").c_str(), &tokenIt, &ruleIt);
            REQUIRE(FG_OK == res);

            AND_THEN("The item's type should be a rule") {
                REQUIRE(FG_RULE_ITEM == item.type);
            }

            AND_THEN("The field rule should point on the first grammar's rule") {
                REQUIRE(g.ruleList.front->data == item.rule);
            }
        }
    }

    GIVEN("An existing rule name") {
        THEN("It should return OK") {
            int res = fg_extractPrItem(&item, std::string("TOKEN1").c_str(), &tokenIt, &ruleIt);
            REQUIRE(FG_OK == res);

            AND_THEN("The item's type should be a token") {
                REQUIRE(FG_TOKEN_ITEM == item.type);
            }

            AND_THEN("The field token should poin on the first grammar's token") {
                REQUIRE(g.tokenList.front->data == item.token);
            }
        }
    }

    ll_freeLinkedList(&itemList, NULL);
    fg_freeGrammar(&g);
}

SCENARIO("A production rule is made by one or more items (token or rule)", "[formal_grammar]") {
    fg_Grammar g = createBasicGrammar();
    ll_LinkedList itemList = ll_createLinkedList();

    ll_LinkedList *productionRule = (ll_LinkedList*) malloc(sizeof(*productionRule));
    productionRule->front = productionRule->back = NULL;
    productionRule->size = 0;

    GIVEN("An iterator on a valid production rule items") {
        ll_pushBackBatch(&itemList, 3, "rule1", "TOKEN1", "|");
        ll_Iterator it = ll_createIterator(&itemList);

        THEN("It should return OK") {
            char *lastItem = NULL;
            int res = fg_extractProductionRule(productionRule, &it, &g, (char*) ll_iteratorNext(&it), &lastItem);

            REQUIRE(FG_OK == res);

            AND_THEN("The production rule should contain 2 items") {
                REQUIRE(2 == productionRule->size);
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
            int res = fg_extractProductionRule(productionRule, &it, &g, (char*) ll_iteratorNext(&it), &lastItem);

            REQUIRE(FG_PRITEM_UNKNOWN_TYPE == res);

            AND_THEN("The production rule should not have any items") {
                REQUIRE(0 == productionRule->size);
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
            int res = fg_extractProductionRule(productionRule, &it, &g, (char*) ll_iteratorNext(&it), &lastItem);

            REQUIRE(FG_PR_EMPTY == res);

            AND_THEN("The production rule should not have any items") {
                REQUIRE(0 == productionRule->size);
            }
        }
    }

    ll_freeLinkedList(&itemList, NULL);
    fg_freeProductionRule(productionRule);
    fg_freeGrammar(&g);
}

SCENARIO("A rule is made by one or more rules separated by a pipe and ends with a semicolon", "[formal_grammar]") {
    fg_Grammar g = createBasicGrammar();
    ll_LinkedList itemList = ll_createLinkedList();

    fg_Rule rule = {};
    memset(&rule, 0, sizeof(rule));

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
    fg_freeRule(&rule);
    fg_freeGrammar(&g);
}
*/