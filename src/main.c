#include "lexer.h"

#include "formal_grammar.h"
#include "log.h"
#include "linked_list.h"
#include "string_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *grammarBuffer = NULL;
    ssize_t grammarSize = lex_readGrammar(stdin, &grammarBuffer);

    if (grammarSize == -1) {
        perror("");
    }

    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, (ll_DataDestructor*) free);
    lex_extractGrammarItems(grammarBuffer, grammarSize, &itemList);

    fg_Grammar g;
    fg_createGrammar(&g);
    int errCode = lex_parseGrammarItems(&g, &itemList);

    if (errCode == FG_OK) {
        log_info("Number of rules : %ld\nNumber of tokens : %ld", g.rules.size, g.tokens.size);
    }
    else {
        log_error("Error code : %d", errCode);
    }

    free(grammarBuffer);
    ll_freeLinkedList(&itemList, NULL);
    fg_freeGrammar(&g);
    return 0;
}
