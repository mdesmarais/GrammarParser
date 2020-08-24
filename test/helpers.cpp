#include "helpers.hpp"

extern "C" {
#include <linked_list.h>
#include <parser.h>
}

void fillItemList(ll_LinkedList *itemList, const std::vector<std::string> &items) {
    for (auto item : items) {
        prs_StringItem *stringItem = (prs_StringItem*) malloc(sizeof(*stringItem));
        stringItem->item = (char*) calloc(item.size() + 1, 1);
        strcpy(stringItem->item, item.c_str());

        stringItem->column = stringItem->line = 0;

        ll_pushBack(itemList, stringItem);
    }
}
