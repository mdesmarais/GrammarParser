#include "hash.h"

#include <assert.h>
#include <string.h>

uint32_t hashString(const char *string) {
    return murmurhash3_32(string, strlen(string));
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
