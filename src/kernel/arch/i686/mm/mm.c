#include <stdbool.h>
#include <stddef.h>

#include "arch/i686/cpu.h"
#include "arch/i686/paging.h"
#include "arch/i686/mm/mm.h"

__attribute__((aligned(0x1000))) pageTableEntry_t mm_temporaryPageTable[1024];
void *mm_temporaryPageTable_base;

void mm_init() {
    pageDirectoryEntry_t *pageDirectory = (pageDirectoryEntry_t *)(((size_t)&kernel_pageDirectory) + 0xc0000000);
    pageDirectoryEntry_t newEntry = {
        .present = 1,
        .writePermission = 1,
        .userPermission = 0,
        .writeThrough = 1,
        .disableCache = 0,
        .accessed = 0,
        .zero = 0,
        .pageSize = 0,
        .available = 0,
        .pageTableAddress = (((size_t)mm_temporaryPageTable) - 0xc0000000) >> 12
    };

    bool found = false;
    
    for(int i = 0; i < 1024 && !found; i++) {
        if(!pageDirectory[i].present) {
            pageDirectory[i] = newEntry;
            mm_temporaryPageTable_base = (void *)(i << 22);
            found = true;
        }
    }

    if(!found) {
        // TODO: die
    }
}

void *mm_mapTemporary(const void *pageAddress, mm_service_t service) {
    void *mappedPageAddress = (void *)(((size_t)mm_temporaryPageTable_base) | (service << 12));

    pageTableEntry_t newEntry = {
        .present = 1,
        .writePermission = 1,
        .userPermission = 0,
        .writeThrough = 1,
        .disableCache = 0,
        .accessed = 0,
        .dirty = 0,
        .zero = 0,
        .global = 0,
        .available = 0,
        .pageAddress = (((size_t)pageAddress) - 0xc0000000) >> 12
    };

    mm_temporaryPageTable[service] = newEntry;

    invlpg(mappedPageAddress);

    return mappedPageAddress;
}

void mm_unmapTemporary(mm_service_t service) {
    void *mappedPageAddress = (void *)(((size_t)mm_temporaryPageTable_base) | (service << 12));

    mm_temporaryPageTable[service].present = 0;

    invlpg(mappedPageAddress);
}
