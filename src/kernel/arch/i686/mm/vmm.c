#include <stdbool.h>
#include <stddef.h>

#include "arch/i686/paging.h"
#include "arch/i686/mm/mm.h"
#include "arch/i686/mm/pmm.h"
#include "libk/string.h"

enum {
    VMM_FLAG_ALLOCATED = 1 << 0,
    VMM_FLAG_SYSTEM = 1 << 1
};

static pageDirectoryEntry_t *vmm_pageDirectory = (pageDirectoryEntry_t *)(((size_t)&kernel_pageDirectory) + 0xc0000000);

void vmm_init();
static void *vmm_map_search(size_t n, bool high);
static int vmm_createPageTable(size_t pageDirectoryIndex);
void *vmm_map(const void *paddr, size_t n, bool high);
void *vmm_map2(const void *paddr, void *vaddr, size_t n);
int vmm_unmap(const void *vaddr, size_t n);
int vmm_unmap2(const void *vaddr, size_t n);
void *vmm_alloc(size_t n, bool high);
int vmm_free(const void *vaddr, size_t n);

void vmm_init() {
    // Transform page directory PSE entries to page table entries
    for(int i = 0; i < 1024; i++) {
        if(vmm_pageDirectory[i].present) {
            if(vmm_pageDirectory[i].pageSize) {
                pageTableEntry_t *newPageTableP = pmm_alloc(1);

                if(newPageTableP == NULL) {
                    // TODO: die
                }

                pageTableEntry_t *newPageTableV = mm_mapTemporary(newPageTableP, MM_SERVICE_VMM);

                size_t address = vmm_pageDirectory[i].pageTableAddress;
                
                pageDirectoryEntry_t pageDirectoryEntry = vmm_pageDirectory[i];

                for(int j = 0; j < 1024; j++) {
                    newPageTableV[j].value = 0;
                    newPageTableV[j].writeThrough = 1;
                    newPageTableV[j].writePermission = 1;
                    newPageTableV[j].present = 1;
                    newPageTableV[j].pageAddress = address + j;
                }

                pageDirectoryEntry.pageTableAddress = ((uint32_t)newPageTableP) >> 12;
                pageDirectoryEntry.pageSize = 0;
                vmm_pageDirectory[i] = pageDirectoryEntry;
            }
        }
    }
}

static void *vmm_map_search(size_t n, bool high) {
    size_t pageDirectoryIndex = high ? 768 : 0;
    int searchLength = high ? 256 : 768;
    size_t pageTableIndex = 0;

    size_t consecutiveFreePageCount = 0;
    size_t firstFreePage_pageDirectoryIndex = pageDirectoryIndex;
    size_t firstFreePage_pageTableIndex = 0;

    for(int i = 0; i < searchLength; i++) {
        if(vmm_pageDirectory[pageDirectoryIndex].present) {
            pageTableEntry_t *pageTable = mm_mapTemporary((const void *)(vmm_pageDirectory[pageDirectoryIndex].pageTableAddress << 12), MM_SERVICE_VMM);

            for(int j = 0; j < 1024; j++) {
                if(pageTable[j].present || (pageTable[j].available & (VMM_FLAG_ALLOCATED | VMM_FLAG_SYSTEM))) {
                    consecutiveFreePageCount = 0;
                } else {
                    if(consecutiveFreePageCount == 0) {
                        firstFreePage_pageDirectoryIndex = pageDirectoryIndex;
                        firstFreePage_pageTableIndex = pageTableIndex;
                    }

                    consecutiveFreePageCount++;
                }

                if(consecutiveFreePageCount >= n) {
                    break;
                } else {
                    pageTableIndex++;

                    if(pageTableIndex == 1024) {
                        pageDirectoryIndex++;
                        pageTableIndex = 0;
                    }
                }
            }
        } else {
            consecutiveFreePageCount += 1024;
        }

        if(consecutiveFreePageCount >= n) {
            return (void *)((firstFreePage_pageDirectoryIndex << 22) | (firstFreePage_pageTableIndex << 12));
        }
    }

    return NULL;
}

static int vmm_createPageTable(size_t pageDirectoryIndex) {
    void *pageTableP = pmm_alloc(1);

    if(!pageTableP) {
        return 1;
    }

    void *pageTableV = mm_mapTemporary(pageTableP, MM_SERVICE_VMM);

    memset(pageTableV, 0, 4096);

    vmm_pageDirectory[pageDirectoryIndex].value = 0;
    vmm_pageDirectory[pageDirectoryIndex].pageTableAddress = ((uint32_t)pageTableP) >> 12;
    vmm_pageDirectory[pageDirectoryIndex].writeThrough = 1;
    vmm_pageDirectory[pageDirectoryIndex].writePermission = 1;
    vmm_pageDirectory[pageDirectoryIndex].present = 1;

    return 0;
}

void *vmm_map2(const void *paddr, void *vaddr, size_t n) {
    size_t pageNumber = ((size_t)paddr) >> 12;
    size_t address = (size_t)vaddr;
    size_t pageDirectoryIndex = address >> 22;
    size_t pageTableIndex = (address >> 12) & 0x3ff;

    if(!vmm_pageDirectory[pageDirectoryIndex].present) {
        if(vmm_createPageTable(pageDirectoryIndex)) {
            return NULL;
        }
    }
    
    pageTableEntry_t *pageTable = mm_mapTemporary((const void *)(vmm_pageDirectory[pageDirectoryIndex].pageTableAddress << 12), MM_SERVICE_VMM);

    for(size_t i = 0; i < n; i++) {
        pageTable[pageTableIndex].pageAddress = pageNumber;
        pageTable[pageTableIndex].global = 0;
        pageTable[pageTableIndex].zero = 0;
        pageTable[pageTableIndex].dirty = 0;
        pageTable[pageTableIndex].accessed = 0;
        pageTable[pageTableIndex].disableCache = 0;
        pageTable[pageTableIndex].writeThrough = 1;
        pageTable[pageTableIndex].writePermission = 1;
        pageTable[pageTableIndex].present = 1;

        pageTableIndex++;
        pageNumber++;

        if(pageTableIndex == 1024) {
            pageTableIndex = 0;
            pageDirectoryIndex++;

            if(!vmm_pageDirectory[pageDirectoryIndex].present) {
                vmm_createPageTable(pageDirectoryIndex);
            } else {
                mm_mapTemporary((const void *)(vmm_pageDirectory[pageDirectoryIndex].pageTableAddress << 12), MM_SERVICE_VMM);
            }
        }
    }

    return vaddr;
}

void *vmm_map(const void *paddr, size_t n, bool high) {
    void *mapAddress = vmm_map_search(n, high);

    if(!mapAddress) {
        return NULL;
    }

    return vmm_map2(paddr, mapAddress, n);
}

int vmm_unmap(const void *vaddr, size_t n) {
    size_t address = (size_t)vaddr;
    size_t pageDirectoryIndex = address >> 22;
    size_t pageTableIndex = (address >> 12) & 0x3ff;

    pageTableEntry_t *pageTable = NULL;

    if(vmm_pageDirectory[pageDirectoryIndex].present) {
        pageTable = mm_mapTemporary((const void *)(vmm_pageDirectory[pageDirectoryIndex].pageTableAddress << 12), MM_SERVICE_VMM);
    }

    for(size_t i = 0; i < n; i++) {
        if(pageTable) {
            pageTable[pageTableIndex].present = 0;
        }

        pageTableIndex++;

        if(pageTableIndex == 1024) {
            pageTableIndex = 0;
            pageDirectoryIndex++;

            if(vmm_pageDirectory[pageDirectoryIndex].present) {
                pageTable = mm_mapTemporary((const void *)(vmm_pageDirectory[pageDirectoryIndex].pageTableAddress << 12), MM_SERVICE_VMM);
            } else {
                pageTable = NULL;
            }
        }
    }

    return 0;
}

int vmm_unmap2(const void *vaddr, size_t n) {
    size_t address = (size_t)vaddr;
    size_t pageDirectoryIndex = address >> 22;
    size_t pageTableIndex = (address >> 12) & 0x3ff;

    pageTableEntry_t *pageTable = NULL;

    if(vmm_pageDirectory[pageDirectoryIndex].present) {
        pageTable = mm_mapTemporary((const void *)(vmm_pageDirectory[pageDirectoryIndex].pageTableAddress << 12), MM_SERVICE_VMM);
    }

    for(size_t i = 0; i < n; i++) {
        if(pageTable) {
            pmm_free((const void *)(pageTable[pageTableIndex].pageAddress << 12), 1);
            pageTable[pageTableIndex].present = 0;
        }

        pageTableIndex++;

        if(pageTableIndex == 1024) {
            pageTableIndex = 0;
            pageDirectoryIndex++;

            if(vmm_pageDirectory[pageDirectoryIndex].present) {
                pageTable = mm_mapTemporary((const void *)(vmm_pageDirectory[pageDirectoryIndex].pageTableAddress << 12), MM_SERVICE_VMM);
            } else {
                pageTable = NULL;
            }
        }
    }

    return 0;
}

void *vmm_alloc(size_t n, bool high) {
    void *vaddr = vmm_map_search(n, high);

    if(vaddr == NULL) {
        return NULL;
    }

    size_t address = (size_t)vaddr;
    size_t pageDirectoryIndex = address >> 22;
    size_t pageTableIndex = (address >> 12) & 0x3ff;

    if(!vmm_pageDirectory[pageDirectoryIndex].present) {
        if(vmm_createPageTable(pageDirectoryIndex)) {
            return NULL;
        }
    }
    
    pageTableEntry_t *pageTable = mm_mapTemporary((const void *)(vmm_pageDirectory[pageDirectoryIndex].pageTableAddress << 12), MM_SERVICE_VMM);

    for(size_t i = 0; i < n; i++) {
        pageTableEntry_t pageTableEntry;

        pageTableEntry.value = 0;
        pageTableEntry.available = VMM_FLAG_ALLOCATED;

        pageTable[pageTableIndex] = pageTableEntry;

        pageTableIndex++;

        if(pageTableIndex == 1024) {
            pageTableIndex = 0;
            pageDirectoryIndex++;

            if(!vmm_pageDirectory[pageDirectoryIndex].present) {
                vmm_createPageTable(pageDirectoryIndex);
            } else {
                mm_mapTemporary((const void *)(vmm_pageDirectory[pageDirectoryIndex].pageTableAddress << 12), MM_SERVICE_VMM);
            }
        }
    }

    return vaddr;
}

int vmm_free(const void *vaddr, size_t n) {
    size_t address = (size_t)vaddr;
    size_t pageDirectoryIndex = address >> 22;
    size_t pageTableIndex = (address >> 12) & 0x3ff;

    while(!vmm_pageDirectory[pageDirectoryIndex].present) {
        if(n > 1024) {
            n -= 1024;
            pageDirectoryIndex++;
        } else {
            return 0;
        }
    }
    
    pageTableEntry_t *pageTable = mm_mapTemporary((const void *)(vmm_pageDirectory[pageDirectoryIndex].pageTableAddress << 12), MM_SERVICE_VMM);

    for(size_t i = 0; i < n; i++) {
        pageTableEntry_t pageTableEntry;

        pageTableEntry.value = 0;
        pageTableEntry.available &= ~VMM_FLAG_ALLOCATED;

        pageTable[pageTableIndex] = pageTableEntry;

        pageTableIndex++;

        if(pageTableIndex == 1024) {
            pageTableIndex = 0;
            pageDirectoryIndex++;

            while(!vmm_pageDirectory[pageDirectoryIndex].present) {
                if(n > 1024) {
                    n -= 1024;
                    pageDirectoryIndex++;
                } else {
                    return 0;
                }
            }

            mm_mapTemporary((const void *)(vmm_pageDirectory[pageDirectoryIndex].pageTableAddress << 12), MM_SERVICE_VMM);
        }
    }

    return 0;
}
