#ifndef __KERNEL_MM_PMM_HPP__
#define __KERNEL_MM_PMM_HPP__

#include "arch/i686/multiboot.hpp"

namespace kernel {
    /**
     *  Summary:
     *      Initializes the physical memory manager.
     * 
     *  Args:
     *      - memoryMap: A pointer to the memory map as passed by GRUB.
     *      - memoryMapLength: The number of entries in the memory map.
     */
    void pmm_init(const multiboot_info_mmap_entry_t *memoryMap, int memoryMapLength);

    /**
     *  Summary:
     *      Allocates a memory page and returns its physical address (paging does
     *      not apply)
     * 
     *  Returns:
     *      The physical memory address of the first byte in the allocated page if
     *      everything went well, or NULL if an error occurred.
     */
    void *pmm_alloc();

    /**
     *  Summary:
     *      Frees a memory page.
     * 
     *  Args:
     *      - addr: A pointer to the physical address of the page.
     */
    void pmm_free(const void *addr);
}

#endif
