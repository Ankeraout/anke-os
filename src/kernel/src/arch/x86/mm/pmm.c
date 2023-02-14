#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/arch.h"
#include "boot/boot.h"
#include "klibc/string.h"
#include "mm/pmm.h"
#include "debug.h"

static uint64_t *s_pmmMap;
static size_t s_pmmMapSize;
static uintptr_t s_pmmMapEndAddress;

static void pmmPrintMemoryMap(
    const struct ts_bootMemoryMapEntry *p_memoryMap,
    size_t p_memoryMapLength
);
static size_t pmmCountPagesInMap(
    const struct ts_bootMemoryMapEntry *p_memoryMap,
    size_t p_memoryMapLength
);
static void pmmFindSpaceForMap(
    const struct ts_bootMemoryMapEntry *p_memoryMap,
    size_t p_memoryMapLength,
    size_t p_nbPages
);
static void pmmMarkAsUsed(
    const void *p_ptr,
    size_t p_size
);

int pmmInit(
    const struct ts_bootMemoryMapEntry *p_memoryMap,
    size_t p_memoryMapLength
) {
    pmmPrintMemoryMap(p_memoryMap, p_memoryMapLength);

    const size_t l_nbPages = pmmCountPagesInMap(
        p_memoryMap,
        p_memoryMapLength
    );

    s_pmmMapSize = (l_nbPages + 7) >> 3;
    const size_t l_mapSizePages = (s_pmmMapSize + 4095) >> 12;

    // Round map size to page granularity
    s_pmmMapSize = l_mapSizePages << 12;
    s_pmmMapEndAddress = l_nbPages << 12;

    pmmFindSpaceForMap(p_memoryMap, p_memoryMapLength, l_mapSizePages);

    if(s_pmmMap == NULL) {
        debugPrint("pmm: Failed to find space for map.\n");
        return 1;
    }

    // Map all pages as used
    memset(s_pmmMap, 0xff, s_pmmMapSize);

    // Free all memory space marked as free or reclaimable
    for(size_t l_index = 0; l_index < p_memoryMapLength; l_index++) {
        if(
            (p_memoryMap[l_index].a_type == E_MMAP_TYPE_FREE)
            || (p_memoryMap[l_index].a_type == E_MMAP_TYPE_RECLAIMABLE)
        ) {
            pmmFree(
                (void *)(uintptr_t)p_memoryMap[l_index].a_base,
                p_memoryMap[l_index].a_size
            );
        }
    }

    // PMM map is considered as used
    pmmMarkAsUsed(s_pmmMap, s_pmmMapSize);

    debugPrint("pmm: Initialization complete.\n");

    return 0;
}

void *pmmAlloc(size_t p_size) {
    const size_t l_pageCount = (p_size + 4095) >> 12;
    uintptr_t l_currentAddress = 0x100000;
    size_t l_consecutiveFreePageCount = 0;
    uintptr_t l_firstFreePage;
    size_t l_entryIndex = l_currentAddress >> 18;
    uint64_t l_entryMask = 1 << ((l_currentAddress >> 12) & 0x3f);

    while(
        (l_consecutiveFreePageCount < l_pageCount)
        && (l_currentAddress < s_pmmMapEndAddress)
    ) {
        const bool l_isPageFree = (s_pmmMap[l_entryIndex] & l_entryMask) == 0;

        if(l_isPageFree) {
            if(l_consecutiveFreePageCount == 0) {
                l_firstFreePage = l_currentAddress;
            }

            l_consecutiveFreePageCount++;
        } else {
            l_consecutiveFreePageCount = 0;
        }

        l_currentAddress += 0x1000;
        l_entryMask <<= 1;

        if(l_entryMask == 0) {
            l_entryMask = 1;
            l_entryIndex++;
        }
    }

    if(l_consecutiveFreePageCount != l_pageCount) {
        return NULL;
    }

    pmmMarkAsUsed((void *)l_firstFreePage, p_size);

    return (void *)l_firstFreePage;
}

void pmmFree(void *p_ptr, size_t p_size) {
    const size_t l_pageNumber = ((size_t)p_ptr) >> 12;
    size_t l_entryIndex = l_pageNumber >> 6;
    uint64_t l_entryMask = 1 << (l_pageNumber & 0x3f);
    size_t l_pageCount = (p_size + 4095) >> 12;

    while(l_pageCount > 0) {
        s_pmmMap[l_entryIndex] &= ~l_entryMask;

        l_entryMask <<= 1;

        if(l_entryMask == 0) {
            l_entryMask = 1;
            l_entryIndex++;
        }

        l_pageCount--;
    }
}

static void pmmPrintMemoryMap(
    const struct ts_bootMemoryMapEntry *p_memoryMap,
    size_t p_memoryMapLength
) {
    debugPrint("pmm: Memory map:\n");

    for(size_t l_index = 0; l_index < p_memoryMapLength; l_index++) {
        debugPrint("pmm: [0x");
        debugPrintHex64(p_memoryMap[l_index].a_base);
        debugPrint("-0x");
        debugPrintHex64(p_memoryMap[l_index].a_base + p_memoryMap[l_index].a_size - 1);
        debugPrint("] : 0x");
        debugPrintHex8(p_memoryMap[l_index].a_type);
        debugPrint("\n");
    }
}

static size_t pmmCountPagesInMap(
    const struct ts_bootMemoryMapEntry *p_memoryMap,
    size_t p_memoryMapLength
) {
    uintptr_t l_highestAddress = 0;

    for(size_t l_index = 0; l_index < p_memoryMapLength; l_index++) {
        if(
            (p_memoryMap[l_index].a_type == E_MMAP_TYPE_FREE)
            || (p_memoryMap[l_index].a_type == E_MMAP_TYPE_KERNEL)
            || (p_memoryMap[l_index].a_type == E_MMAP_TYPE_RECLAIMABLE)
        ) {
            const uintptr_t l_entryHighestAddress =
                p_memoryMap[l_index].a_base + p_memoryMap[l_index].a_size;

            if(l_entryHighestAddress > l_highestAddress) {
                l_highestAddress = l_entryHighestAddress;
            }
        }
    }

    const size_t l_nbPages = l_highestAddress >> 12;

    return l_nbPages;
}

static void pmmFindSpaceForMap(
    const struct ts_bootMemoryMapEntry *p_memoryMap,
    size_t p_memoryMapLength,
    size_t p_nbPages
) {
    size_t l_index = 0;

    while((l_index < p_memoryMapLength) && (s_pmmMap == NULL)) {
        if(
            (p_memoryMap[l_index].a_type == E_MMAP_TYPE_FREE)
            && (p_memoryMap[l_index].a_size >= p_nbPages * 4096)
            && (p_memoryMap[l_index].a_base >= 0x100000)
        ) {
            s_pmmMap = (uint64_t *)(uintptr_t)p_memoryMap[l_index].a_base;
        }

        l_index++;
    }
}

static void pmmMarkAsUsed(
    const void *p_ptr,
    size_t p_size
) {
    const size_t l_pageNumber = ((size_t)p_ptr) >> 12;
    size_t l_entryIndex = l_pageNumber >> 6;
    uint64_t l_entryMask = 1 << (l_pageNumber & 0x3f);
    size_t l_pageCount = (p_size + 4095) >> 12;

    while(l_pageCount > 0) {
        s_pmmMap[l_entryIndex] |= l_entryMask;

        l_entryMask <<= 1;

        if(l_entryMask == 0) {
            l_entryMask = 1;
            l_entryIndex++;
        }

        l_pageCount--;
    }
}
