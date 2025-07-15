#ifndef __INCLUDE_MEMORYALLOCATION_H__
#define __INCLUDE_MEMORYALLOCATION_H__

#include <stddef.h>

#include "mm/vmm.h"
#include "list.h"

struct ts_memoryAllocation {
    void *m_ptr;
    size_t m_size;
};

/**
 * @brief Allocates memory in the context of a process.
 * 
 * @param[in] p_context The context of the process.
 * @param[inout] p_list The list of allocated physical memory sections.
 * @param[in] p_size The size of the memory to allocate.
 * 
 * @returns A pointer to the virtual address of the allocated memory.
 * @retval NULL if an error occurred.
 */
void *memoryAllocation_allocate(
    struct ts_vmm_context *p_context,
    struct ts_list_node **p_list,
    size_t p_size
);

#endif
