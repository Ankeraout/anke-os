#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "debug.hpp"
#include "panic.hpp"
#include "arch/i686/io.hpp"
#include "arch/i686/multiboot.hpp"
#include "libk/libk.hpp"
#include "mm/mm.hpp"
#include "mm/pmm.hpp"
#include "mm/vmm.hpp"

#define VIRTUAL_TO_PHYSICAL_ADDR(address) ((address) - 0xc0000000)

namespace kernel {
    // When we don't care about the type
    typedef int whatever_t;

    // Declaration of the kernel page directory
    extern "C" uint32_t kernel_pageDirectory[1024];

    // Kernel start symbol
    extern "C" whatever_t __kernel_start;

    // Kernel end symbol
    extern "C" whatever_t __kernel_end;

    // First free page number or -1 if none is available
    int pmm_freePageNumber = -1;

    void pmm_init(const multiboot_info_mmap_entry_t *memoryMap, int memoryMapLength) {
        // TODO: unmap the page table from page directory at the end

        bool freeSpaceFound = false;

        // Look for a free space in the page directory
        for(int i = 0x3ff; i >= 0x300; i--) {
            if(!(kernel_pageDirectory[i] & 0x00000001)) {
                // Register page table
                // Write-through, Read/Write, Present
                kernel_pageDirectory[i] = VIRTUAL_TO_PHYSICAL_ADDR((uint32_t)mm_pageTable) | 0x0000000b;
                mm_pageTableIndex = i;
                freeSpaceFound = true;
                break;
            }
        }

        // If we could not register the page table, panic
        if(!freeSpaceFound) {
            panic("pmm_init(): Could not register page table.");
        }

        debug("\n\nSystem memory map:\n");
        debug("Base             | End              | Type\n");
        debug("-----------------+------------------+-----\n");

        // Free all usable
        for(int i = 0; i < memoryMapLength; i++) {
            char buffer[100];

            debug(std::hex64(memoryMap[i].base_addr, buffer));
            debug(" | ");
            debug(std::hex64(memoryMap[i].base_addr + memoryMap[i].length - 1, buffer));
            debug(" | ");
            debug(std::itoa(memoryMap[i].type, buffer, 10));
            debug("\n");

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

                    // Don't mark kernel pages or pages < 1MB as free
                    if(
                        (
                            (pageNumber >= kernelStartPageNumber)
                            && (pageNumber <= kernelEndPageNumber)
                        )
                        || pageNumber < 0x100
                    ) {
                        continue;
                    }

                    pmm_free((void *)pageAddress32);
                }
            }
        }

        debug("\n");
    }

    void *pmm_alloc() {
        // TODO: don't use mm_pageTable. Use a page table allocated by vmm instead

        if(pmm_freePageNumber == -1) {
            // No pages to allocate
            return NULL;
        }

        // Compute the address of the first page of the page table
        uint32_t address = mm_pageTableIndex << 22;

        // Save the allocated page number
        int allocatedPageNumber = pmm_freePageNumber;

        // Compute the new page address
        uint32_t allocatedPageAddress = allocatedPageNumber << 12;

        // Map the page
        mm_pageTable[0] = allocatedPageAddress | 0x0000000b;

        // Invalidate the corresponding page
        invlpg((void *)address);

        // Extract the next page number from it
        pmm_freePageNumber = *((int *)address);

        // Clean the page
        std::memset((void *)address, 0, MM_PAGE_SIZE);

        // Unmap the page
        mm_pageTable[0] = 0;

        // Invalidate the corresponding page
        invlpg((void *)address);

        // Return the address as a pointer
        return (void *)allocatedPageAddress;
    }

    void pmm_free(const void *addr) {
        // TODO: don't use mm_pageTable. Use a page table allocated by vmm instead
        // Note that this function is used by pmm_init(), and in this case it needs
        // mm_pageTable.

        // Compute the page number of the page to free
        int pageNumber = ((uint32_t)addr) >> 12;

        // Compute the address of the first page of the page table
        uint32_t address = mm_pageTableIndex << 22;

        // Map the page
        mm_pageTable[0] = (pageNumber << 12) | 0x0000000b;
        
        // Invalidate the corresponding page
        invlpg((void *)address);

        // Save the free page number in it
        *((int *)address) = pmm_freePageNumber;

        // Unmap the page
        mm_pageTable[0] = 0;

        // Invalidate the page
        invlpg((void *)address);

        // Save the page number
        pmm_freePageNumber = pageNumber;
    }
}
