#ifndef __MM_H__
#define __MM_H__

#include <stdint.h>

// Page table reserved for mm use
extern uint32_t mm_pageTable[1024];

// Page table index in the page directory
extern int mm_pageTableIndex;

int mm_init();

#endif
