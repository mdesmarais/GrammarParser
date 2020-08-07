#include "unit.h"

#include <lexer.h>
#include <linked_list.h>
#include <log.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/*static void test_createLetterRange() {
    lex_Range r1;

    int result = lex_createLetterRange(&r1, '$', 'f', false);
    ASSERT_INT_EQUALS("The first character is not a valid letter, it should return an error", LEXER_INVALID_CHAR_RANGE, result);

    result = lex_createLetterRange(&r1, 'f', '$', false);
    ASSERT_INT_EQUALS("The second character is not a valid letter, it should return an error", LEXER_INVALID_CHAR_RANGE, result);

    result = lex_createLetterRange(&r1, 'm', 'f', false);
    ASSERT_INT_EQUALS("The second character is before the first one, it should return an error", LEXER_INVALID_RANGE, result);

    result = lex_createLetterRange(&r1, 'x', 'z', false);
    ASSERT_INT_EQUALS("Given characters are valid, it should return OK", LEXER_OK, result);

    ASSERT_CHAR_EQUALS("lex_Range should have its first pointer on char x", 'x', *r1.start);
    ASSERT_CHAR_EQUALS("lex_Range should have its second pointer on char z (-1)", 'z', *(r1.end - 1));
}

void test_matchInRange() {
    lex_Range r1, r2;
    int init1 = lex_createLetterRange(&r1, 'd', 'f', false);
    // r2 is used to test behavior with null pointer
    // pointer to the end the range will is to null
    int init2 = lex_createLetterRange(&r2, 'x', 'z', false);

    ASSERT_INT_EQUALS("lex_Range init 1", LEXER_OK, init1);
    ASSERT_INT_EQUALS("lex_Range init 2", LEXER_OK, init2);

    bool result = lex_matchInRange(&r1, 'b', true);
    ASSERT_FALSE("Given character is out of range (before), it should return false", result);

    result = lex_matchInRange(&r2, '5', true);
    ASSERT_FALSE("Given a character which is out of range (number), it should return false", result);

    result = lex_matchInRange(&r1, 'd', true);
    ASSERT_TRUE("Given a character which is the start of the range, it should return true", result);

    result = lex_matchInRange(&r1, 'e', true);
    ASSERT_TRUE("Given a character which is in the range, it should return true", result);

    result = lex_matchInRange(&r1, 'f', true);
    ASSERT_TRUE("Given a character which is the end of the range, it should return true", result);
}

static void test_extractRange() {
    lex_Range r1;
    int result = lex_extractRange(&r1, "abc");
    ASSERT_INT_EQUALS("Given an invalid pattern, it should return LEXER_INVALID_RANGE_PATTERN", LEXER_INVALID_RANGE_PATTERN, result);

    result = lex_extractRange(&r1, "a-7");
    ASSERT_INT_EQUALS("Given an invalid pattern, it should return LEXER_INVALID_RANGE_PATTERN", LEXER_INVALID_RANGE_PATTERN, result);

    result = lex_extractRange(&r1, "a-b");
    ASSERT_INT_EQUALS("Given a valid letter range pattern, it should return ok", LEXER_OK, result);
    ASSERT_CHAR_EQUALS("lex_Range should have its first pointer on char a", 'a', *r1.start);
    ASSERT_CHAR_EQUALS("lex_Range should have its second pointer on char b (-1)", 'b', *(r1.end - 1));

    lex_Range r2;
    result = lex_extractRange(&r2, "A-B");
    ASSERT_INT_EQUALS("Given a valid uppercase letter range pattern, it should return ok", LEXER_OK, result);
    ASSERT_CHAR_EQUALS("lex_Range should have its first pointer on char a", 'a', *r2.start);
    ASSERT_CHAR_EQUALS("lex_Range should have its second pointer on char b (-1)", 'b', *(r2.end - 1));
    ASSERT_TRUE("Uppercase boolean should be set to true", r2.uppercaseLetter);

    lex_Range r3;
    result = lex_extractRange(&r3, "1-5");
    ASSERT_INT_EQUALS("Given a valid digit range pattern, it should return ok", LEXER_OK, result);
    ASSERT_CHAR_EQUALS("lex_Range should have its first pointer on char 1", '1', *r3.start);
    ASSERT_CHAR_EQUALS("lex_Range should have its second pointer on char 2", '5', *(r3.end - 1));
}

static void test_extractRanges() {
    lex_Range *ranges = NULL;
    int extractedRanges = lex_extractRanges(&ranges, "", 0);

    ASSERT_INT_EQUALS("Given an empty string, it should return 0", 0, extractedRanges);
    ASSERT_FALSE("The input pointer should stay unchanged (null)", ranges);

    extractedRanges = lex_extractRanges(&ranges, "a-zA-Z0-9", 9);

    ASSERT_INT_EQUALS("Given a string with 3 ranges, it should return 3", 3, extractedRanges);
    ASSERT_FALSE("The input pointer should have been modified", ranges == NULL);
    free(ranges);
    ranges = NULL;

    extractedRanges = lex_extractRanges(&ranges, "a-z@-d1-3", 9);
    ASSERT_INT_EQUALS("Given a string with 2 invalid patterns, it should return -1", -1, extractedRanges);
    ASSERT_FALSE("The input pointer should stay unchanged (null)", ranges);

    extractedRanges = lex_extractRanges(&ranges, "a-zb", 4);
    ASSERT_INT_EQUALS("Given a string with one pattern and an additional char, it should return -1", -1, extractedRanges);
    ASSERT_FALSE("The input pointer should stay unchanged (null)", ranges);
}*/

static void test_extractGrammarItems() {
    char *input1 = "op=      NUMBER \n"
                   "\t| SUB NUMBER \n"
                   "\t| `hello world`;";

    ll_LinkedList itemList = ll_createLinkedList();
    int extractedItems = lex_extractGrammarItems(input1, strlen(input1), &itemList);

    ASSERT_INT_EQUALS("pwet", 9, extractedItems);

    ll_LinkedList expected = ll_createLinkedList();
    ll_pushBackBatch(&expected, 9, "op", "=", "NUMBER", "|", "SUB", "NUMBER", "|", "`hello world`", ";");

    ASSERT_LIST_EQUALS("Checking if all extracted items equal the expected list", expected, itemList, strcmp);

    ll_freeLinkedList(&expected, NULL);
    ll_freeLinkedList(&itemList, free);
}

int main() {
    /*test_createLetterRange();
    test_matchInRange();
    test_extractRange();
    test_extractRanges();*/
    test_extractGrammarItems();
    return 0;
}