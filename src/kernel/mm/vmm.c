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
static inline uint32_t vmm_getPageTableEntry(const void *addr) {
    uint32_t address32 = (uint32_t)addr;
    uint32_t pageDirectoryIndex = address32 >> 22;
    uint32_t pageTableIndex = (address32 >> 12) & 0x000003ff;

    if(kernel_pageDirectory[pageDirectoryIndex] & 0x01) {
        // Present flag set in page directory entry
        
    } else {
        return 0;
    }
}

int vmm_map(const void *paddr, const void *vaddr) {

}

int vmm_unmap(const void *vaddr) {

}

void *vmm_alloc(int pageCount) {

}

void vmm_free(const void *addr) {

}
