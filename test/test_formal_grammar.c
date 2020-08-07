#include "unit.h"

#include <formal_grammar.h>
#include <linked_list.h>
#include <log.h>

#include <stdlib.h>
#include <string.h>

#define createListAndIt(list, it, itemsNumber, ...) do { \
    (list) = ll_createLinkedList();                      \
    ll_pushBackBatch(&(list), (itemsNumber), __VA_ARGS__); \
    (it) = ll_createIterator(&(list));                   \
} while(0);

static void test_extractToken() {
    ll_LinkedList itemList;
    ll_Iterator it;

    createListAndIt(itemList, it, 3, "TOKEN", "=", ";");
    fg_Token token1 = { 0 };
    int res = fg_extractToken(&token1, &it, ll_iteratorNext(&it));
    ASSERT_INT_EQUALS("Given a token without a value, it should return an error", FG_TOKEN_MISSING_VALUE, res);

    ll_freeLinkedList(&itemList, NULL);
    createListAndIt(itemList, it, 4, "HI", "=", "`hello`", ";");
    fg_Token token2 = { 0 };
    res = fg_extractToken(&token2, &it, ll_iteratorNext(&it));

    ASSERT_INT_EQUALS("Given valid items for a string token, it should return 0", 0, res);
    ASSERT_STR_EQUALS("The token's name should be HI", "HI", token2.name);
    ASSERT_INT_EQUALS("The token's type should be FG_STRING_TOKEN", FG_STRING_TOKEN, token2.type);
    ASSERT_STR_EQUALS("The token's string should be hello", "hello", token2.string);

    fg_freeToken(&token2);
    ll_freeLinkedList(&itemList, NULL);
}

int main() {
    test_extractToken();
    return 0;
}