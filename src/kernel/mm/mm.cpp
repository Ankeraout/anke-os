#include <stdint.h>

#include "mm/mm.hpp"

namespace kernel {
    uint32_t mm_pageTable[1024] __attribute__((aligned(0x1000)));

    int mm_pageTableIndex = 0;
}
