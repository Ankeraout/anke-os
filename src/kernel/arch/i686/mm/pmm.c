#include <stdbool.h>
#include <stddef.h>

#include "arch/i686/mmap.h"
#include "arch/i686/mm/mm.h"
#include "libk/string.h"

static __attribute__((aligned(0x1000))) uint8_t pmm_memoryPageTable[32 * 4096];
static size_t pmm_highestAvailablePageNumber;
static size_t pmm_lowestAvailablePageNumber = UINT32_MAX;

extern int __kernel_start;
extern int __kernel_end;

void pmm_init();
void *pmm_alloc(int n);
void pmm_free(const void *pages, int n);
static void pmm_mark(size_t pageNumber, int n, bool used);

void pmm_init() {
    memset(pmm_memoryPageTable, 0xff, 32 * 4096);

    const mmap_entry_t *mmap = mmap_get();
    size_t mmap_length = mmap_getLength();

    for(size_t i = 0; i < mmap_length; i++) {
        if(mmap[i].type == 1) {
            uint64_t roundedBase = (mmap[i].base + 0xfff) & 0xfffffffffffff000;
            uint64_t roundedLength = (mmap[i].length - (roundedBase - mmap[i].base)) & 0xfffffffffffff000;
            uint64_t end = roundedBase + roundedLength;

            if(roundedBase < end) {
                pmm_free((const void *)(size_t)roundedBase, roundedLength >> 12);
                pmm_highestAvailablePageNumber = (end - 1) >> 12;

                size_t pageNumber = roundedBase >> 12;

                if(pmm_lowestAvailablePageNumber > pageNumber) {
                    pmm_lowestAvailablePageNumber = pageNumber;
                }
            }
        }
    }

    // Mark kernel pages as used
    size_t kernel_size = ((size_t)&__kernel_end) - ((size_t)&__kernel_start);
    size_t kernel_pages = (kernel_size + 0xfff) >> 12;
    size_t pageNumber = (((size_t)&__kernel_start) - 0xc0000000) >> 12;
    pmm_mark(pageNumber, kernel_pages, true);

    // Mark < 1MB addresses as used
    pmm_mark(0, 256, true);
}

void *pmm_alloc(int n) {
    if(n == 0) {
        return NULL;
    }

    static size_t pageNumber = 0;
    size_t pageCount = 0;
    size_t firstAvailablePage = pageNumber;

    size_t firstPageNumber = pageNumber;
    size_t memoryTablePageIndex = pageNumber >> 3;
    size_t memoryTablePageByteShift = pageNumber & 0x07;

    do {
        // Get page entry
        uint8_t pageEntry = pmm_memoryPageTable[memoryTablePageIndex] & (0x80 >> memoryTablePageByteShift);
        
        if(pageEntry) {
            pageCount = 0;
        } else {
            if(pageCount == 0) {
                firstAvailablePage = pageNumber;
            }

            pageCount++;
        }

        memoryTablePageByteShift++;

        if(memoryTablePageByteShift == 8) {
            memoryTablePageIndex++;
            memoryTablePageByteShift = 0;
        }

        pageNumber++;

        if((pageCount < (size_t)n) && (pageNumber > pmm_highestAvailablePageNumber)) {
            pageCount = 0;
            pageNumber = pmm_lowestAvailablePageNumber;
            memoryTablePageIndex = pageNumber >> 3;
            memoryTablePageByteShift = pageNumber & 0x07;
        }
    } while((pageNumber != firstPageNumber) && (pageCount < (size_t)n));

    if(pageCount == (size_t)n) {
        pmm_mark(firstAvailablePage, pageCount, true);
        return (void *)(firstAvailablePage << 12);
    } else {
        return NULL;
    }
}

void pmm_free(const void *pages, int n) {
    size_t pageNumber = ((size_t)pages) >> 12;
    pmm_mark(pageNumber, n, false);
}

static void pmm_mark(size_t pageNumber, int n, bool used) {
    size_t memoryTablePageIndex = pageNumber >> 3;
    size_t memoryTablePageByteShift = pageNumber & 0x07;

    for(int i = 0; i < n; i++) {
        if(used) {
            pmm_memoryPageTable[memoryTablePageIndex] |= 0x80 >> memoryTablePageByteShift;
        } else {
            pmm_memoryPageTable[memoryTablePageIndex] &= ~(0x80 >> memoryTablePageByteShift);
        }
        
        memoryTablePageByteShift++;

        if(memoryTablePageByteShift == 8) {
            memoryTablePageIndex++;
            memoryTablePageByteShift = 0;
        }
    }
}
