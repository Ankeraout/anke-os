#include <stddef.h>
#include <stdint.h>

#include "panic.h"
#include "arch/i686/io.h"
#include "libk/libk.h"
#include "mm/mm.h"
#include "mm/pmm.h"
#include "mm/vmm.h"

#define VIRTUAL_TO_PHYSICAL_ADDR(address) ((address) - 0xc0000000)

// When we don't care about the type
typedef int whatever_t;

// Declaration of the kernel page directory
extern uint32_t kernel_pageDirectory[1024];

/**
 *  Summary:
 *      Returns the page table entry for the given virtual memory address. If
 *      there is no page table mapped for this address in the page directory,
 *      then this function will return 0.
 * 
 *  Args:
 *      - addr: The virtual address for which to get the page table entry.
 * 
 *  Returns:
 *      The page table entry for the given virtual address.
 */
uint32_t vmm_getPageTableEntry(const void *addr) {
    uint32_t address32 = (uint32_t)addr;
    uint32_t pageDirectoryIndex = address32 >> 22;
    uint32_t pageTableIndex = (address32 >> 12) & 0x000003ff;
    uint32_t *pageTable = (uint32_t *)((mm_pageTableIndex << 22) | 0x00001000);

    // Present flag set in page directory entry
    if(kernel_pageDirectory[pageDirectoryIndex] & 0x00000001) {
        // Map the page table to the temporary area
        mm_pageTable[1] = (kernel_pageDirectory[pageDirectoryIndex] & 0xfffff000) | 0x0000000b;

        // Invalidate the TLB cache
        invlpg(pageTable);

        // Read the value
        uint32_t pageTableEntryValue = pageTable[pageTableIndex];

        // Unmap the page table from the temporary area
        mm_pageTable[1] = 0;

        // Invalidate the TLB cache
        invlpg(pageTable);

        // Return the value
        return pageTableEntryValue;
    } else {
        return 0;
    }
}

int vmm_map(const void *paddr, const void *vaddr) {
    uint32_t address32 = (uint32_t)vaddr;
    uint32_t pageDirectoryIndex = address32 >> 22;
    uint32_t pageTableIndex = (address32 >> 12) & 0x000003ff;
    uint32_t *pageTable = (uint32_t *)((mm_pageTableIndex << 22) | 0x00001000);

    // Check if there is a page directory entry for this address. If none,
    // create a new page table and map it.
    if(!(kernel_pageDirectory[pageDirectoryIndex] & 0x00000001)) {
        // Allocate a new page for the new page table
        void *newPageTable_p = pmm_alloc();

        if(!newPageTable_p) {
            // Page allocation failed
            return 1;
        }

        // Map the page table to the temporary area
        mm_pageTable[1] = (((uint32_t)newPageTable_p) & 0xfffff000) | 0x0000000b;

        // Invalidate the TLB cache
        invlpg(pageTable);

        // Clear the page table
        memset(pageTable, 0, 0x1000);

        // Map the page in the page table
        pageTable[pageTableIndex] = (((uint32_t)paddr) & 0xfffff000) | 0x0000000b;

        // Unmap the page table from the temporary area
        mm_pageTable[1] = 0;

        // Invalidate the TLB cache
        invlpg(pageTable);

        // Map the page table in the page directory
        kernel_pageDirectory[pageDirectoryIndex] = ((uint32_t)newPageTable_p) | 0x0000000b;
    } else {
        // Map the page table to the temporary area
        mm_pageTable[1] = (kernel_pageDirectory[pageDirectoryIndex] & 0xfffff000) | 0x0000000b;

        // Invalidate the TLB cache
        invlpg(pageTable);

        // Check the present flag in the page table entry
        if(pageTable[pageTableIndex] & 0x00000001) {
            // Unmap the page table from the temporary area
            mm_pageTable[1] = 0;

            // Invalidate the TLB cache
            invlpg(pageTable);
            
            // A page was already mapped here
            return 1;
        }

        // Map the page in the page table
        pageTable[pageTableIndex] = (((uint32_t)paddr) & 0xfffff000) | 0x0000000b;

        // Unmap the page table from the temporary area
        mm_pageTable[1] = 0;

        // Invalidate the TLB cache
        invlpg(pageTable);
    }

    // Invalidate the TLB cache
    invlpg(vaddr);

    // No errors occurred.
    return 0;
}

int vmm_unmap(const void *vaddr) {
    uint32_t address32 = (uint32_t)vaddr;
    uint32_t pageDirectoryIndex = address32 >> 22;
    uint32_t pageTableIndex = (address32 >> 12) & 0x000003ff;
    uint32_t *pageTable = (uint32_t *)((mm_pageTableIndex << 22) | 0x00001000);

    // Present flag set in page directory entry
    if(kernel_pageDirectory[pageDirectoryIndex] & 0x00000001) {
        // Map the page table to the temporary area
        mm_pageTable[1] = (kernel_pageDirectory[pageDirectoryIndex] & 0xfffff000) | 0x0000000b;

        // Invalidate the TLB cache
        invlpg(pageTable);

        int returnValue = 0;

        if(pageTable[pageTableIndex] & 0x00000001) {
            pageTable[pageTableIndex] = 0;
        } else {
            returnValue = 1;
        }

        // Unmap the page table from the temporary area
        mm_pageTable[1] = 0;

        // Invalidate the TLB cache
        invlpg(pageTable);
        invlpg(vaddr);

        // Return the value
        return returnValue;
    } else {
        return 1;
    }
}

void *vmm_alloc(int pageCount) {
    // TODO
    UNUSED_PARAMETER(pageCount);
    return NULL;
}

void vmm_free(const void *addr) {
    // TODO
    UNUSED_PARAMETER(addr);
}
