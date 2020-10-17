#include <stdint.h>

#include "mm/mm.h"

uint32_t mm_pageTable[1024] __attribute__((aligned(0x1000)));

int mm_pageTableIndex = 0;
