#include <catch2/catch.hpp>

#include "helpers.hpp"

#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>

extern "C" {
#include <collections/linked_list.h>
#include <formal_grammar.h>
#include <hash.h>
#include <parser.h>
}

using Catch::Matchers::Equals;

static int stringItemCmp(const prs_StringItem *d1, const prs_StringItem *d2) {
    return strcmp(d1->item, d2->item);
}

SCENARIO("Items can be extracted from a raw grammar", "[parser]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, (ll_DataDestructor*) prs_freeStringItem);

    GIVEN("A string containing a rule with several production rules") {
        std::string input = "%op=      NUMBER \n"
                            "\t| SUB NUMBER \n"
                            "\t| `hello world`;";

        int extractedItems = prs_extractGrammarItems(input.c_str(), input.size(), &itemList);

        THEN("9 items should have been extracted") {
            REQUIRE(9 == extractedItems);

            ll_LinkedList expected;
            ll_createLinkedList(&expected, (ll_DataDestructor*) prs_freeStringItem);

            fillItemList(&expected, { "%op", "=", "NUMBER", "|", "SUB", "NUMBER", "|", "`hello world`", ";" });

            REQUIRE(ll_isEqual(&itemList, &expected, (ll_DataComparator*) stringItemCmp));

            ll_freeLinkedList(&expected, nullptr);
        }
    }

    ll_freeLinkedList(&itemList, nullptr);
}

SCENARIO("Read a grammar file into a dynamic buffer", "[parser]") {
    GIVEN("A sample file") {
        FILE *f = fopen("data/test_01.txt", "r");
        REQUIRE(f);

        GIVEN("A null buffer") {
            char *buffer = nullptr;

            ssize_t length = prs_readGrammar(f, &buffer);

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
    }
}

SCENARIO("Extract tokens and rules from a list of items", "[parser]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, (ll_DataDestructor*) prs_freeStringItem);

    fg_Grammar g;
    fg_createGrammar(&g);

    GIVEN("A list with a valid token and an unknown item") {
        fillItemList(&itemList, { "%TOKEN1", "=", "[a-z]", ";", "`hello`", ";" });

        int res = prs_parseGrammarItems(&g, &itemList);

        THEN("It should return an error") {
            REQUIRE(PRS_UNKNOWN_ITEM == res);
        }
    }

    GIVEN("A list with 1 rule and 1 token") {
        fillItemList(&itemList, { "%TOKEN1", "=", "`hello`", ";", "%rule1", "=", "TOKEN1", ";" });

        int res = prs_parseGrammarItems(&g, &itemList);

        THEN("It should return OK") {
            REQUIRE(PRS_OK == res);
        }

        AND_THEN("The entry rule should be rule1") {
            auto rule1 = (fg_Rule*) ht_getValue(&g.rules, "rule1");

            REQUIRE(rule1);
            REQUIRE(g.entry == rule1);
        }

        AND_WHEN("Parsing an existing rule") {
            ll_freeLinkedList(&itemList, nullptr);
            fillItemList(&itemList, { "%rule1", "=", "rule1", ";" });

            res = prs_parseGrammarItems(&g, &itemList);

            THEN("It should return an error") {
                REQUIRE(FG_RULE_EXISTS == res);
            }
        }

        AND_WHEN("Parsing an existing token") {
            ll_freeLinkedList(&itemList, nullptr);
            fillItemList(&itemList, { "%TOKEN1", "=", "`already exists`", ";" });

            res = prs_parseGrammarItems(&g, &itemList);

            THEN("it should return an error") {
                REQUIRE(FG_TOKEN_EXISTS == res);
            }
        }
    }

    GIVEN("A list with an invalid token") {
        fillItemList(&itemList, { "%TOKEN1", "=", ";" });

        int res = prs_parseGrammarItems(&g, &itemList);

        THEN("It should return an error") {
            REQUIRE_FALSE(PRS_OK == res);
        }
    }

    GIVEN("A list with an invalid rule") {
        fillItemList(&itemList, { "%rule", "TOKEN", ";" });

        int res = prs_parseGrammarItems(&g, &itemList);

        THEN("It should return an error") {
            REQUIRE_FALSE(PRS_OK == res);
        }
    }

    fg_freeGrammar(&g);
    ll_freeLinkedList(&itemList, nullptr);
}

SCENARIO("symbols resolution is used to allow recursive rules", "[parser]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, (ll_DataDestructor*) prs_freeStringItem);

    fg_Grammar g;
    fg_createGrammar(&g);

    GIVEN("A token that uses an unknown token") {
        fillItemList(&itemList, { "%TOKEN1", "=", "TOKEN2", ";" });

        int res = prs_parseGrammarItems(&g, &itemList);
        REQUIRE(res == PRS_OK);

        WHEN("resolving symbols") {
            res = prs_resolveSymbols(&g);

            THEN("It should return an error") {
                REQUIRE(res == FG_UNKNOWN_TOKEN);
            }
        }
    }

    GIVEN("A rule that uses an unknown rule") {
        fillItemList(&itemList, { "%rule1", "=", "rule2", ";" });

        int res = prs_parseGrammarItems(&g, &itemList);
        REQUIRE(res == PRS_OK);

        WHEN("resolving symbols") {
            res = prs_resolveSymbols(&g);

            THEN("It should return an error") {
                REQUIRE(res == FG_UNKNOWN_RULE);
            }
        }
    }

    GIVEN("A rule that uses an unknown token") {
        fillItemList(&itemList, { "%rule1", "=", "TOKEN1", ";" });

        int res = prs_parseGrammarItems(&g, &itemList);
        REQUIRE(res == PRS_OK);

        WHEN("resolving symbols") {
            res = prs_resolveSymbols(&g);

            THEN("It should return an error") {
                REQUIRE(res == FG_UNKNOWN_TOKEN);
            }
        }
    }

    GIVEN("Two rules : one is using the other and a token") {
        fillItemList(&itemList, { "%rule1", "=", "rule1", "TOKEN1", "|", "rule2", ";",
                         "%TOKEN1", "=", "[a-z]", ";", "%rule2", "=", "TOKEN1", ";" });

        int res = prs_parseGrammarItems(&g, &itemList);
        REQUIRE(res == PRS_OK);

        WHEN("resolving symbols") {
            res = prs_resolveSymbols(&g);

            THEN("It should return OK") {
                REQUIRE(PRS_OK == res);
            }

            AND_THEN("Resolution on rule1's production rule should have be done") {
                auto rule1 = (fg_Rule*) ht_getValue(&g.rules, "rule1");

                ll_Iterator it = ll_createIterator((ll_LinkedList*) rule1->productionRuleList.front->data);

                // Testing recursive rule
                auto prItem1 = (fg_PRItem*) ll_iteratorNext(&it);
                REQUIRE(rule1 == prItem1->value.rule);

                auto prItem2 = (fg_PRItem*) ll_iteratorNext(&it);
                auto token1 = (fg_Token*) ht_getValue(&g.tokens, "TOKEN1");
                REQUIRE(token1 == prItem2->value.token);
            }
        }
    }

    GIVEN("Two tokens : one is referencing the other") {
        fillItemList(&itemList, { "%TOKEN1", "=", "TOKEN2", ";", "%TOKEN2", "=", "`test`", ";" });

        int res = prs_parseGrammarItems(&g, &itemList);
        REQUIRE(res == PRS_OK);

        WHEN("Resolving symbols") {
            res = prs_resolveSymbols(&g);

            THEN("It should return ok") {
                REQUIRE(PRS_OK == res);
            }

            AND_THEN("TOKEN1 should have a reference on TOKEN2") {
                auto token1 = (fg_Token*) ht_getValue(&g.tokens, "TOKEN1");
                auto token2 = (fg_Token*) ht_getValue(&g.tokens, "TOKEN2");

                REQUIRE(token1);
                REQUIRE(token2);

                REQUIRE(token2 == token1->value.refToken.token);
            }
        }
    }

    fg_freeGrammar(&g);
    ll_freeLinkedList(&itemList, nullptr);
}

SCENARIO("string items have a position (line, column) in a source string", "[parser]") {
    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, (ll_DataDestructor*) prs_freeStringItem);

    fillItemList(&itemList, { "TOKEN1", "`hello`", ";" });

    ll_Iterator it = ll_createIterator(&itemList);

    GIVEN("A random source string without any token or rule") {
        const char *source = "hello world";

        THEN("It should return false") {
            REQUIRE_FALSE(prs_computeItemsPosition(source, &it));
        }
    }

    GIVEN("A string with all items, separated by spaces and new lines") {
        const char *source = " TOKEN1\n\n`hello`;";
        bool res = prs_computeItemsPosition(source, &it);

        THEN("It should return true") {
            REQUIRE(res);
        }

        AND_THEN("Each string item's position should have been update") {
            it = ll_createIterator(&itemList);

            auto item = (prs_StringItem*) ll_iteratorNext(&it);
            REQUIRE(1 == item->line);
            REQUIRE(2 == item->column);

            item = (prs_StringItem*) ll_iteratorNext(&it);
            REQUIRE(3 == item->line);
            REQUIRE(1 == item->column);

            item = (prs_StringItem*) ll_iteratorNext(&it);
            REQUIRE(3 == item->line);
            REQUIRE(8 == item->column);
        }
    }

    ll_freeLinkedList(&itemList, nullptr);
}

SCENARIO("Computes a set of probable first terminals", "[parser]") {
    ht_Table table;
    ht_createTable(&table, 10, (ht_HashFunction*) hashProductionRule, nullptr, [](void *key, void *value) {
        auto set = (set_HashSet *) value;
        set_freeSet(set);
        free(set);
    });

    ll_LinkedList pr;
    ll_createLinkedList(&pr, nullptr);

    GIVEN("An empty hash table") {
        GIVEN("A string pr item") {
            fg_PRItem prItem = { .type = FG_STRING_ITEM };
            prItem.value.string = (char*) "+";
            ll_pushBack(&pr, &prItem);

            set_HashSet *set = prs_first(&table, &pr, &prItem, nullptr);

            THEN("The output set should have a size of one") {
                REQUIRE(1 == set->size);
            }

            AND_THEN("The element should be a string") {
                auto parserItem = firstSetItem<prs_ParserItem>(set);
                REQUIRE(PRS_STRING_ITEM == parserItem->type);
                REQUIRE_THAT("+", Equals(parserItem->value.string));
            }
        }

        GIVEN("A token pr item with a string token") {
            fg_Token token = { .type = FG_STRING_TOKEN, .name = (char*) "TOKEN"};
            token.value.string = (char*) "+";

            prs_StringItem stringItem = { .item = (char*) "TOKEN" };

            fg_PRItem prItem = { .type = FG_TOKEN_ITEM, .symbol = &stringItem };
            prItem.value.token = &token;
            ll_pushBack(&pr, &prItem);

            bool isOptional;
            set_HashSet *set = prs_first(&table, &pr, &prItem, &isOptional);

            THEN("The output set should have a size of one") {
                REQUIRE(1 == set->size);
            }

            AND_THEN("The element should be a string") {
                auto parserItem = firstSetItem<prs_ParserItem>(set);
                REQUIRE(PRS_STRING_ITEM == parserItem->type);
                REQUIRE_THAT("+", Equals(parserItem->value.string));
            }

            AND_THEN("The given pr item should not be optional") {
                REQUIRE_FALSE(isOptional);
            }

            AND_WHEN("Token is optional") {
                token.quantifier = PRS_STAR_QUANTIFIER;

                set = prs_first(&table, &pr, &prItem, &isOptional);

                THEN("Given boolean should be true") {
                    REQUIRE(isOptional);
                }
            }
        }

        GIVEN("A rule pr item with a string token and a range token") {
            ll_LinkedList itemList;
            ll_createLinkedList(&itemList, (ll_DataDestructor*) prs_freeStringItem);
            fillItemList(&itemList, {
                "%TOKEN1", "=", "`+`", ";",
                "%TOKEN2", "=", "[a-t]", ";",
                "%rule1", "=", "TOKEN1", "|", "TOKEN2", ";",
                "%rule2", "=", "rule1", ";"
            });

            fg_Grammar g;
            fg_createGrammar(&g);

            REQUIRE(PRS_OK == prs_parseGrammarItems(&g, &itemList));
            REQUIRE(PRS_OK == prs_resolveSymbols(&g));

            auto rule1 = (fg_Rule*) ht_getValue(&g.rules, "rule1");

            auto rule2 = (fg_Rule*) ht_getValue(&g.rules, "rule2");

            auto pouet = (ll_LinkedList*) rule1->productionRuleList.front->data;
            auto prItem = (fg_PRItem*) pouet->front->data;
            set_HashSet *set = prs_first(&table, pouet, prItem, nullptr);


            auto pouet2 = (ll_LinkedList*) rule1->productionRuleList.front->data;
            auto prItem2 = (fg_PRItem*) pouet->front->data;
            set_HashSet *set2 = prs_first(&table, pouet2, prItem2, nullptr);

            set_union(set, set2); // @FIXME the problem is here
            REQUIRE(2 == set->size);

            fg_freeGrammar(&g);
            ll_freeLinkedList(&itemList, nullptr);
        }

        GIVEN("A ref token") {
            fg_Token t1 = { .type = FG_STRING_TOKEN, .name = (char*) "TOKEN1"};
            t1.value.string = (char*) "+";

            fg_Token t2 = { .type = FG_REF_TOKEN, .name = (char*) "TOKEN2"};
            t2.value.refToken.token = &t1;

            prs_StringItem stringItem = { .item = (char*) "TOKEN2" };

            fg_PRItem prItem = { .type = FG_TOKEN_ITEM, .symbol = &stringItem };
            prItem.value.token = &t2;
            ll_pushBack(&pr, &prItem);

            set_HashSet *set = prs_first(&table, &pr, &prItem, nullptr);

            THEN("The result set should contain only one element") {
                REQUIRE(1 == set->size);
            }

            AND_THEN("The element should be a parser item +") {
                auto parserItem = firstSetItem<prs_ParserItem>(set);
                REQUIRE(PRS_STRING_ITEM == parserItem->type);
                REQUIRE_THAT("+", Equals(parserItem->value.string));
            }
        }

        ll_freeLinkedList(&pr, NULL);
        ht_freeTable(&table);
    }

}

SCENARIO("pouet", "[parser]") {
    ht_Table table;
    ht_createTable(&table, 10, (ht_HashFunction *) hashProductionRule, nullptr, [](void *key, void *value) {
        auto set = (set_HashSet *) value;
        set_freeSet(set);
        free(set);
    });

    ll_LinkedList pr;
    ll_createLinkedList(&pr, nullptr);

    GIVEN("A pr with two required item") {
        fg_PRItem prItem1 = { .type = FG_STRING_ITEM };
        prItem1.value.string = (char*) "hello";

        fg_PRItem prItem2 = { .type = FG_STRING_ITEM };
        prItem2.value.string = (char*) "world";

        ll_pushBackBatch(&pr, 2, &prItem1, &prItem2);

        set_HashSet *set = prs_prFirst(&table, &pr);

        THEN("The result set should contain only one element") {
            REQUIRE(1 == set->size);
        }
    }

    GIVEN("A pr with one optional item and one required") {
        fg_Token token = { .type = FG_STRING_TOKEN, .name = (char*) "TOKEN" };
        token.value.string = (char*) "foo";
        token.quantifier = PRS_STAR_QUANTIFIER;

        prs_StringItem stringItem = { .item = (char*) "TOKEN" };
        fg_PRItem prItem1 = { .type = FG_TOKEN_ITEM, .symbol = &stringItem };
        prItem1.value.token = &token;

        fg_PRItem prItem2 = { .type = FG_STRING_ITEM };
        prItem2.value.string = (char*) "hello";

        ll_pushBackBatch(&pr, 2, &prItem1, &prItem2);

        set_HashSet *set = prs_prFirst(&table, &pr);

        THEN("The result set should contain two elements") {
            REQUIRE(2 == set->size);
        }
    }

    ll_freeLinkedList(&pr, NULL);
    ht_freeTable(&table);
}
