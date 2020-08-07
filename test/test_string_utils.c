#include "unit.h"

#include <linked_list.h>
#include <log.h>
#include <string_utils.h>

#include <stdlib.h>
#include <string.h>

static void test_findDelimiter() {
    char *input1 = "hello+worl-d";
    char *input2 = "hello world";
    char *delimiters = "+-";

    const char *delim = str_findDelimiter(input1, strlen(input1), delimiters, 2);

    ASSERT_PTR_EQUALS("Given a string with 2 delimiters, it should return ptr on the + char", input1 + 5, delim);

    delim = str_findDelimiter(input2, strlen(input2), delimiters, 2);
    ASSERT_PTR_EQUALS("Given a string without any delimiter, it should return null", NULL, delim);
}

static void test_removeWhitespaces() {
    char *input = "   ";
    char target[3] = { '\0' };

    size_t result = str_removeWhitespaces(target, input, strlen(input));
    ASSERT_INT_EQUALS("Given a string full of whitespaces, it should return 0", 0, result);
    ASSERT_INT_EQUALS("The target buffer should remain empty", 0, strlen(target));

    char *input2 = "  a  t  lldf ";
    char target2[12] = { '\0' };

    result = str_removeWhitespaces(target2, input2, strlen(input2));
    ASSERT_INT_EQUALS("It should have removed 6 spaces, it should return 6", 6, result);
    ASSERT_INT_EQUALS("pwet", 6, strlen(target2));
}

static void test_removeMultipleSpaces() {
    char *input1 = " \n\t";
    char target1[3] = { '\0' };

    str_removeMultipleSpaces(target1, input1, strlen(input1));
    ASSERT_INT_EQUALS("Given a string with 3 spaces, it should set target with only one char", 1, strlen(target1));
    ASSERT_CHAR_EQUALS("The only character should be a whitespace", ' ', *target1);

    char *input2 = "  hello world  ";
    char target2[15] = { '\0' };

    str_removeMultipleSpaces(target2, input2, strlen(input2));
    ASSERT_INT_EQUALS("Given a string with pre and sufix equal to 2 spaces, it should remove 2 spaces", 13, strlen(target2));
}

static void test_splitItems() {
    char *input1 = ",hello,world,,, ";

    ll_LinkedList itemList = ll_createLinkedList();

    ssize_t itemsNumber = str_splitItems(input1, strlen(input1), &itemList, ',');

    ASSERT_INT_EQUALS("Given a string with 3 items, it should return 3", 3, itemsNumber);
    ASSERT_INT_EQUALS("The size of the list should be 3", 3, itemList.size);

    ll_LinkedListItem *item1 = itemList.front;
    ll_LinkedListItem *item2 = item1->next;
    ll_LinkedListItem *item3 = item2->next;

    ASSERT_STR_EQUALS("Checking first item", "hello", item1->data);
    ASSERT_STR_EQUALS("Checking second item", "world", item2->data);
    ASSERT_STR_EQUALS("Checking third item", " ", item3->data);

    ll_freeLinkedList(&itemList, free);
}

int main() {
    test_findDelimiter();
    test_removeWhitespaces();
    test_removeMultipleSpaces();
    test_splitItems();
    return 0;
}