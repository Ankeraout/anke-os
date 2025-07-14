#ifndef __INCLUDE_MM_PMM_H__
#define __INCLUDE_MM_PMM_H__

#include <stddef.h>
#include <stdint.h>

#include "mm/mm.h"

/**
 * @brief Initializes the physical memory manager.
 * 
 * @param[in] p_memoryMap An array of memory map entries, representing the
 * memory map of the system.
 * @param[in] s_memoryMapEntryCount The number of entries in the memory map.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success
*/
int pmm_init(
    const struct ts_mm_memoryMapEntry *p_memoryMap,
    int s_memoryMapEntryCount
);

/**
 * @brief Requests a chunk of physical memory.
 * 
 * @param[in] p_size The minimum size of the physical memory chunk to allocate.
 * 
 * @returns A pointer to the allocated physical memory chunk, or NULL if the
 * allocation failed.
*/
void *pmm_alloc(size_t p_size);

/**
 * @brief Frees a chunk of physical memory.
 * 
 * @param[in] p_ptr The address of the physical memory chunk to free.
 * @param[in] p_size The size of the physical memory chunk.
*/
void pmm_free(void *p_ptr, size_t p_size);

#endif
