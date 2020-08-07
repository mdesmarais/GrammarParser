#include "lexer.h"

#include "log.h"
#include "linked_list.h"
#include "string_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void itemHandler(void *data, void *params) {
    log_debug("Item : %s", (char*) data);
}

int main() {
    char *grammarBuffer = NULL;
    ssize_t grammarSize = lex_readGrammar(stdin, &grammarBuffer);

    if (grammarSize == -1) {
        perror("");
    }

    ll_LinkedList itemList = ll_createLinkedList();
    lex_extractGrammarItems(grammarBuffer, grammarSize, &itemList);

    ll_forEachItem(&itemList, itemHandler, NULL);

    free(grammarBuffer);
    ll_freeLinkedList(&itemList, free);
    return 0;
}
