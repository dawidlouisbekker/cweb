#include <stdint.h>
#include <stdio.h>

#define CACHE_LINE_SIZE 64
/*
struct CacheFriendly {
    uint64_t id;
    uint64_t timestamp;
    uint32_t flags;
    uint32_t counter;
    uint8_t  status;

    // padding to fill the rest of the cache line
    uint8_t  _pad[CACHE_LINE_SIZE - (8 + 8 + 4 + 4 + 1)];
} __attribute__((aligned(64)));
 */

const char* cwebNames[];
const char* cWebPaths[];


//struct 
