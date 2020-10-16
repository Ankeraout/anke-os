#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/i686/io.h"
#include "arch/i686/multiboot.h"
#include "mm/pmm.h"
#include "panic.h"
#include "libk/libk.h"

#define VIRTUAL_TO_PHYSICAL_ADDR(address) ((address) - 0xc0000000)

// When we don't care about the type
typedef int whatever_t;

// Declaration of the kernel page directory
extern uint32_t kernel_pageDirectory[1024];

// Kernel start symbol
extern whatever_t __kernel_start;

// Kernel end symbol
extern whatever_t __kernel_end;

// Page table reserved for pmm use
uint32_t pmm_pageTable[1024] __attribute__((aligned(0x1000)));

// Page table index in the page directory
int pmm_pageTableIndex = 0;

// First free page number or -1 if none is available
int pmm_freePageNumber = -1;

void pmm_init(const multiboot_info_mmap_entry_t *memoryMap, int memoryMapLength) {
    bool freeSpaceFound = false;

    // Look for a free space in the page directory
    for(int i = 0; i < 1024; i++) {
        if(!(kernel_pageDirectory[i] & 0x00000001)) {
            // Register page table
            // Write-through, Read/Write, Present
            kernel_pageDirectory[i] = VIRTUAL_TO_PHYSICAL_ADDR((uint32_t)pmm_pageTable) | 0x0000000b;
            pmm_pageTableIndex = i;
            freeSpaceFound = true;
            break;
        }
    }

    // If we could not register the page table, panic
    if(!freeSpaceFound) {
        kernel_panic("pmm_init(): Could not register page table.");
    }

    // Free all usable
    for(int i = 0; i < memoryMapLength; i++) {
        // Useless to try to map reserved or >4GiB RAM
        if(
            (memoryMap[i].type == 1)
            && (memoryMap[i].base_addr < 0x100000000)
        ) {
            for(
                uint64_t pageAddress = memoryMap[i].base_addr;
                pageAddress < memoryMap[i].base_addr + memoryMap[i].length;
                pageAddress += 0x1000
            ) {
                // Useless to try to map RAM > 4GiB
                if(pageAddress > 0x100000000) {
                    break;
                }

                // We now know the page address is on 32 bits
                uint32_t pageAddress32 = (uint32_t)pageAddress;

                // Compute the page number
                uint32_t pageNumber = pageAddress32 >> 12;

                // Compute kernel start page number
                uint32_t kernelStartPageNumber = VIRTUAL_TO_PHYSICAL_ADDR((uint32_t)&__kernel_start) >> 12;

                // Compute kernel end page number
                uint32_t kernelEndPageNumber = VIRTUAL_TO_PHYSICAL_ADDR((uint32_t)&__kernel_end) >> 12;

                // If the kernel finishes on a page boundary
                if((((uint32_t)&__kernel_end) & 0x00000fff) == 0) {
                    kernelEndPageNumber--;
                }

                // Don't mark kernel pages as free
                if(
                    (pageNumber >= kernelStartPageNumber)
                    && (pageNumber <= kernelEndPageNumber)
                ) {
                    continue;
                }

                pmm_free((void *)pageAddress32);
            }
        }
    }
}

void *pmm_alloc() {
    if(pmm_freePageNumber == -1) {
        // No pages to allocate
        return NULL;
    }

    // Compute the address of the first page of the page table
    uint32_t address = pmm_pageTableIndex << 22;

    // Save the allocated page number
    int allocatedPageNumber = pmm_freePageNumber;

    // Compute the new page address
    uint32_t allocatedPageAddress = allocatedPageNumber << 12;

    // Map the page
    pmm_pageTable[0] = allocatedPageAddress | 0x0000000b;

    // Invalidate the corresponding page
    invlpg((void *)address);

    // Extract the next page number from it
    pmm_freePageNumber = *((int *)address);

    // Unmap the page
    pmm_pageTable[0] = 0;

    // Invalidate the corresponding page
    invlpg((void *)address);

    // Return the address as a pointer
    return (void *)allocatedPageAddress;
}

void pmm_free(const void *addr) {
    // Compute the page number of the page to free
    int pageNumber = ((uint32_t)addr) >> 12;

    // Compute the address of the first page of the page table
    uint32_t address = pmm_pageTableIndex << 22;

    // Map the page
    pmm_pageTable[0] = (pageNumber << 12) | 0x0000000b;
    
    // Invalidate the corresponding page
    invlpg((void *)address);

    // Save the free page number in it
    *((int *)address) = pmm_freePageNumber;

    // Unmap the page
    pmm_pageTable[0] = 0;

    // Invalidate the page
    invlpg((void *)address);

    // Save the page number
    pmm_freePageNumber = pageNumber;
}
