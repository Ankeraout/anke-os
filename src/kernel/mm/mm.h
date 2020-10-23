#ifndef __MM_H__
#define __MM_H__

#include <stdint.h>

#define MM_PAGE_SIZE 4096

enum {
    MM_PAGETABLEINDEX_PMM = 0,
    MM_PAGETABLEINDEX_VMM = 1
};

// Page table reserved for mm use
extern uint32_t mm_pageTable[1024];

// Page table index in the page directory
extern int mm_pageTableIndex;

#endif
