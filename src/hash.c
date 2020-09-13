#include "hash.h"

#include "formal_grammar.h"
#include "parser.h"

#include <assert.h>
#include <string.h>

uint32_t hashProductionRule(ll_LinkedList *pr) {
    ll_Iterator it = ll_createIterator(pr);
    uint32_t hash = murmurhash3_32(&pr->size, sizeof(pr->size));

    while (ll_iteratorHasNext(&it)) {
        fg_PRItem *prItem = ll_iteratorNext(&it);

        hash = 3 * hash + hashPRItem(prItem);
    }

    return hash;
}

uint32_t hashPRItem(const fg_PRItem *prItem) {
    assert(prItem);

    uint32_t h = murmurhash3_32(&prItem->type, sizeof(prItem->type));

    switch (prItem->type) {
        case FG_STRING_ITEM:
            return 3 * h + hashString(prItem->value.string);
        case FG_RULE_ITEM:
        case FG_TOKEN_ITEM:
            return 3 * h + hashStringItem(prItem->symbol);
    }
}

uint32_t prs_hashParserItem(const prs_ParserItem *parserItem) {
    assert(parserItem);

    uint32_t h = murmurhash3_32(&parserItem->type, sizeof(parserItem->type));

    switch (parserItem->type) {
        case PRS_RANGE_ITEM: {
            const prs_RangeArray *rangeArray = &parserItem->value.rangeArray;
            h = 3 * rangeArray->size;

            for (size_t i = 0;i < rangeArray->size;++i) {
                prs_Range *range = rangeArray->ranges + i;

                uint32_t h2 = murmurhash3_32(&range->start, sizeof(range->start));
                h2 = 3 * h2 + murmurhash3_32(&range->end, sizeof(range->end));
                h2 = 3 * h2 + murmurhash3_32(&range->uppercaseLetter, sizeof(&range->uppercaseLetter));

                h = 3 * h + h2;
            }
        }
        case PRS_STRING_ITEM:
            return 3 * h + hashString(parserItem->value.string);
    }
}

uint32_t hashString(const char *string) {
    return murmurhash3_32(string, strlen(string));
}

uint32_t hashStringItem(const prs_StringItem *stringItem) {
    uint32_t h = murmurhash3_32(&stringItem->column, sizeof(stringItem->column));
    h = 3 * h + murmurhash3_32(&stringItem->line, sizeof(stringItem->line));
    return 3 * h + hashString(stringItem->item);
}

static uint32_t rot132(uint32_t x, int8_t r) {
    return (x << r) | (x >> (32 - r));
}

uint32_t murmurhash3_32(const void *key, size_t len) {
    assert(key);

    const uint8_t * data = (const uint8_t*)key;
    const int nblocks = len / 4;
    int i;

    uint32_t h = 0;

    uint32_t c1 = 0xcc9e2d51;
    uint32_t c2 = 0x1b873593;

    //----------
    // body

    const uint32_t * blocks = (const uint32_t *)(data + nblocks*4);

    for(i = -nblocks; i; i++)
    {
        uint32_t k1 = blocks[i];

        k1 *= c1;
        k1 = rot132(k1,15);
        k1 *= c2;

        h ^= k1;
        h = rot132(h,13);
        h = h*5+0xe6546b64;
    }

    //----------
    // tail

    const uint8_t * tail = (const uint8_t*)(data + nblocks*4);

    uint32_t k1 = 0;

    switch(len & 3)
    {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
            k1 *= c1; k1 = rot132(k1,15); k1 *= c2; h ^= k1;
    };

    //----------
    // finalization

    h ^= len;

    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}
