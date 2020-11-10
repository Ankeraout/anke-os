#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "panic.h"
#include "arch/i686/io.h"
#include "libk/libk.h"
#include "mm/mm.h"
#include "mm/pmm.h"
#include "mm/vmm.h"

#define PAGETABLE_FLAG_PRESENT 0x00000001
#define PAGETABLE_FLAG_MAPPED PAGETABLE_FLAG_PRESENT
#define PAGETABLE_FLAG_READWRITE 0x00000002
#define PAGETABLE_FLAG_USER 0x00000004
#define PAGETABLE_FLAG_WRITETHROUGH 0x00000008
#define PAGETABLE_FLAG_LASTMAP 0x00000200
#define PAGETABLE_FLAG_ALLOCATED 0x00000400
#define PAGETABLE_FLAG_LASTALLOC 0x00000800
#define PAGEDIRECTORY_FLAG_PRESENT PAGETABLE_FLAG_PRESENT

#define VIRTUAL_TO_PHYSICAL_ADDR(address) ((address) - 0xc0000000)

// When we don't care about the type
typedef int whatever_t;

// Declaration of the kernel page directory
extern uint32_t kernel_pageDirectory[1024];

static inline void invalidateTemporary() {
    invlpg((void *)((mm_pageTableIndex << 22) | (MM_PAGETABLEINDEX_VMM << 12)));
}

static inline void vmm_mapTemporary(void *paddr) {
    mm_pageTable[MM_PAGETABLEINDEX_VMM] = ((uint32_t)paddr & 0xfffff000) | 0x0000000b;
    invalidateTemporary();
}

static inline void vmm_unmapTemporary() {
    mm_pageTable[MM_PAGETABLEINDEX_VMM] = 0;
    invalidateTemporary();
}

void *vmm_map(const void *paddr, size_t n) {
    void *vaddr = vmm_alloc(n);

    if(!vaddr) {
        return NULL;
    }

    if(!vmm_map2(paddr, vaddr, n)) {
        vmm_free(vaddr);
        return NULL;
    } else {
        return vaddr;
    }
}

static inline int vmm_map2_checkPageDirectoryEntryExists(size_t pageDirectoryIndex) {
    if(!(kernel_pageDirectory[pageDirectoryIndex] & PAGEDIRECTORY_FLAG_PRESENT)) {
        void *newPageTable = pmm_alloc();

        if(!newPageTable) {
            return 1;
        } else {
            vmm_mapTemporary(newPageTable);

            kernel_pageDirectory[pageDirectoryIndex] = ((uint32_t)newPageTable) | 0x0000000f;
        }
    }

    return 0;
}

void *vmm_map2(const void *paddr, void *vaddr, size_t n) {
    size_t pageDirectoryIndex = ((size_t)vaddr) >> 22;
    size_t pageTableIndex = (((size_t)vaddr) >> 12) & 0x3ff;
    uint32_t *pageTable = (uint32_t *)((mm_pageTableIndex << 22) | (MM_PAGETABLEINDEX_VMM << 12));

    if(vmm_map2_checkPageDirectoryEntryExists(pageDirectoryIndex)) {
        return NULL;
    }

    for(size_t i = 0; i < n; i++) {
        pageTable[pageTableIndex] &= 0x00000fff;
        pageTable[pageTableIndex] |= (((uint32_t)paddr + i * MM_PAGE_SIZE) & 0xfffff000) | PAGETABLE_FLAG_MAPPED | PAGETABLE_FLAG_READWRITE | PAGETABLE_FLAG_USER | PAGETABLE_FLAG_WRITETHROUGH;

        if(i == n - 1) {
            pageTable[pageTableIndex] |= PAGETABLE_FLAG_LASTMAP;
        }

        invlpg((const void *)((uint32_t)vaddr + i * MM_PAGE_SIZE));

        pageTableIndex++;

        if(pageTableIndex == 1024) {
            pageTableIndex = 0;
            pageDirectoryIndex++;
            
            if(vmm_map2_checkPageDirectoryEntryExists(pageDirectoryIndex)) {
                vmm_unmap2(vaddr, i + 1);
                return NULL;
            }
        }

        if(pageDirectoryIndex == 1024) {
            pageDirectoryIndex = 0;
            
            if(vmm_map2_checkPageDirectoryEntryExists(pageDirectoryIndex)) {
                vmm_unmap2(vaddr, i + 1);
                return NULL;
            }
        }
    }

    return vaddr;
}

static inline int vmm_unmap_checkPageDirectoryEntryExists(size_t pageDirectoryIndex) {
    if(!(kernel_pageDirectory[pageDirectoryIndex] & PAGEDIRECTORY_FLAG_PRESENT)) {
        return 1;
    }

    vmm_mapTemporary((void *)kernel_pageDirectory[pageDirectoryIndex]);

    return 0;
}

int vmm_unmap(const void *vaddr) {
    size_t pageDirectoryIndex = ((size_t)vaddr) >> 22;
    size_t pageTableIndex = (((size_t)vaddr) >> 12) & 0x3ff;
    uint32_t *pageTable = (uint32_t *)((mm_pageTableIndex << 22) | (MM_PAGETABLEINDEX_VMM << 12));
    bool lastUnmapped = false;

    if(vmm_unmap_checkPageDirectoryEntryExists(pageDirectoryIndex)) {
        // Error: we didn't find "last map" flag
        return 1;
    }

    while(!lastUnmapped) {
        if(pageTable[pageTableIndex] & PAGETABLE_FLAG_LASTMAP) {
            lastUnmapped = true;
        }

        pageTable[pageTableIndex] = 0;

        invlpg((const void *)vaddr);

        if(!lastUnmapped) {
            vaddr += MM_PAGE_SIZE;

            pageTableIndex++;

            if(pageTableIndex == 1024) {
                pageTableIndex = 0;
                pageDirectoryIndex++;
                
                if(vmm_unmap_checkPageDirectoryEntryExists(pageDirectoryIndex)) {
                    // Error: we didn't find "last map" flag
                    return 1;
                }
            }

            if(pageDirectoryIndex == 1024) {
                pageDirectoryIndex = 0;
                
                if(vmm_unmap_checkPageDirectoryEntryExists(pageDirectoryIndex)) {
                    // Error: we didn't find "last map" flag
                    return 1;
                }
            }
        }
    }

    return 0;
}

int vmm_unmap2(const void *vaddr, size_t n) {
    size_t pageDirectoryIndex = ((size_t)vaddr) >> 22;
    size_t pageTableIndex = (((size_t)vaddr) >> 12) & 0x3ff;
    uint32_t *pageTable = (uint32_t *)((mm_pageTableIndex << 22) | (MM_PAGETABLEINDEX_VMM << 12));
    bool lastUnmapped = false;
    size_t unmappedPageCount = 0;

    if(vmm_unmap_checkPageDirectoryEntryExists(pageDirectoryIndex)) {
        // Error: we didn't find "last map" flag
        return 1;
    }

    while(unmappedPageCount < n) {
        if(pageTable[pageTableIndex] & PAGETABLE_FLAG_LASTMAP) {
            lastUnmapped = true;
        }

        unmappedPageCount++;

        pageTable[pageTableIndex] = 0;

        invlpg((const void *)vaddr);

        if((!lastUnmapped) && (unmappedPageCount < n)) {
            vaddr += MM_PAGE_SIZE;

            pageTableIndex++;

            if(pageTableIndex == 1024) {
                pageTableIndex = 0;
                pageDirectoryIndex++;
                
                if(vmm_unmap_checkPageDirectoryEntryExists(pageDirectoryIndex)) {
                    // Error: we didn't find "last map" flag
                    return 1;
                }
            }

            if(pageDirectoryIndex == 1024) {
                pageDirectoryIndex = 0;
                
                if(vmm_unmap_checkPageDirectoryEntryExists(pageDirectoryIndex)) {
                    // Error: we didn't find "last map" flag
                    return 1;
                }
            }
        }

        if(lastUnmapped && (unmappedPageCount < n)) {
            // Error: we unmapped less than asked
            return 1;
        }
    }

    return 0;
}

static inline void *vmm_alloc_findFreeBlock(size_t n) {
    uint32_t *pageTable = (uint32_t *)((mm_pageTableIndex << 22) | (MM_PAGETABLEINDEX_VMM << 12));
    size_t address = 0xc0000000;
    size_t pageDirectoryIndex = address >> 22;
    size_t pageTableIndex = (address >> 12) & 0x3ff;
    size_t freePages = 0;

    bool found = false;

    while((!found) && (pageDirectoryIndex < 1024)) {
        if(!(kernel_pageDirectory[pageDirectoryIndex] & PAGEDIRECTORY_FLAG_PRESENT)) {
            freePages += 1024;
        } else {
            vmm_mapTemporary((void *)kernel_pageDirectory[pageDirectoryIndex]);

            while((pageTableIndex < 1024) && (freePages < n)) {
                if(!(pageTable[pageTableIndex] & PAGETABLE_FLAG_PRESENT)) {
                    freePages++;
                } else {
                    freePages = 0;
                    address = ((pageDirectoryIndex << 22) | (pageTableIndex << 12)) + MM_PAGE_SIZE;
                }

                pageTableIndex++;
            }
        }

        if(freePages >= n) {
            found = true;
        } else {
            pageTableIndex = 0;
            pageDirectoryIndex++;
        }
    }

    if(found) {
        return (void *)address;
    } else {
        return NULL;
    }
}

void *vmm_alloc(size_t n) {
    // Look for a free block at least n pages wide
    void *vaddr = vmm_alloc_findFreeBlock(n);

    if(!vaddr) {
        return NULL;
    }

    // Mark all pages as allocated
    size_t pageDirectoryIndex = ((size_t)vaddr) >> 22;
    size_t pageTableIndex = (((size_t)vaddr) >> 12) & 0x3ff;
    uint32_t *pageTable = (uint32_t *)((mm_pageTableIndex << 22) | (MM_PAGETABLEINDEX_VMM << 12));

    if(vmm_map2_checkPageDirectoryEntryExists(pageDirectoryIndex)) {
        return NULL;
    }

    for(size_t i = 0; i < n; i++) {
        pageTable[pageTableIndex] |= PAGETABLE_FLAG_ALLOCATED;

        if(i == n - 1) {
            pageTable[pageTableIndex] |= PAGETABLE_FLAG_LASTALLOC;
        }

        pageTableIndex++;

        if(pageTableIndex == 1024) {
            pageTableIndex = 0;
            pageDirectoryIndex++;
            
            if(vmm_map2_checkPageDirectoryEntryExists(pageDirectoryIndex)) {
                vmm_free(vaddr);
                return NULL;
            }
        }

        if(pageDirectoryIndex == 1024) {
            pageDirectoryIndex = 0;
            
            if(vmm_map2_checkPageDirectoryEntryExists(pageDirectoryIndex)) {
                vmm_free(vaddr);
                return NULL;
            }
        }
    }

    return vaddr;
}

int vmm_free(const void *vaddr) {
    size_t pageDirectoryIndex = ((size_t)vaddr) >> 22;
    size_t pageTableIndex = (((size_t)vaddr) >> 12) & 0x3ff;
    uint32_t *pageTable = (uint32_t *)((mm_pageTableIndex << 22) | (MM_PAGETABLEINDEX_VMM << 12));
    bool lastFreed = false;

    if(vmm_unmap_checkPageDirectoryEntryExists(pageDirectoryIndex)) {
        // Error: we didn't find "last map" flag
        return 1;
    }

    while(!lastFreed) {
        // Make sure the entry is allocated
        if(!(pageTable[pageTableIndex] & PAGETABLE_FLAG_ALLOCATED)) {
            // Error: tried to free an unallocated space
            return 1;
        }

        if(pageTable[pageTableIndex] & PAGETABLE_FLAG_LASTALLOC) {
            lastFreed = true;
        }

        pageTable[pageTableIndex] &= ~(PAGETABLE_FLAG_LASTALLOC | PAGETABLE_FLAG_ALLOCATED | PAGETABLE_FLAG_LASTMAP | PAGETABLE_FLAG_MAPPED);

        invlpg((const void *)vaddr);

        if(!lastFreed) {
            vaddr += MM_PAGE_SIZE;

            pageTableIndex++;

            if(pageTableIndex == 1024) {
                pageTableIndex = 0;
                pageDirectoryIndex++;
                
                if(vmm_unmap_checkPageDirectoryEntryExists(pageDirectoryIndex)) {
                    // Error: we didn't find "last alloc" flag
                    return 1;
                }
            }

            if(pageDirectoryIndex == 1024) {
                pageDirectoryIndex = 0;
                
                if(vmm_unmap_checkPageDirectoryEntryExists(pageDirectoryIndex)) {
                    // Error: we didn't find "last alloc" flag
                    return 1;
                }
            }
        }
    }

    return 0;
}
