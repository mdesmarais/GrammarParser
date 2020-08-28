#include "parser.h"

#include "parser_errors.h"
#include "formal_grammar.h"
#include "log.h"
#include "linked_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *grammarBuffer = NULL;
    log_info("Loading grammar");
    ssize_t grammarSize = prs_readGrammar(stdin, &grammarBuffer);

    if (grammarSize == -1) {
        perror("");
        return 0;
    }

    log_info("Done.");

    ll_LinkedList itemList;
    ll_createLinkedList(&itemList, (ll_DataDestructor *) prs_freeStringItem);

    log_info("Extracting grammar items");
    prs_extractGrammarItems(grammarBuffer, grammarSize, &itemList);

    ll_Iterator it = ll_createIterator(&itemList);
    prs_computeItemsPosition(grammarBuffer, &it);
    log_info("Done.");

    log_info("Parsing items");

    fg_Grammar g;
    fg_createGrammar(&g);
    int errCode = prs_parseGrammarItems(&g, &itemList);

    char errMsg[255];

    if (errCode != PRS_OK) {
        prs_getErrorMessage(errMsg, 255, errCode);
        log_error(errMsg);
        goto clean;
    }

    log_info("Done.\nResolving symbols");
    errCode = prs_resolveSymbols(&g);

    if (errCode == PRS_OK) {
        log_info("Done.");
        log_info("Extracted tokens : %d\nExtracted rules :%d", g.tokens.size, g.rules.size);
    }
    else {
        log_error("Error during resolution : %d", errCode);
        prs_getErrorMessage(errMsg, 255, errCode);
        log_error(errMsg);
        goto clean;
    }

clean:
    free(grammarBuffer);
    ll_freeLinkedList(&itemList, NULL);
    fg_freeGrammar(&g);
    return 0;
}
