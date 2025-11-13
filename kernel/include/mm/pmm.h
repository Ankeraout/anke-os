#ifndef __INCLUDE_MM_PMM_H__
#define __INCLUDE_MM_PMM_H__

#include <stddef.h>
#include <stdint.h>

#include "mm/mm.h"

/**
 * @brief Initializes the physical memory manager.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success
*/
int pmm_init(void);

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

/**
 * @brief Pointer to the HHDM.
 */
extern void *g_pmm_hhdm;

/**
 * @brief Converts a linear pointer to a physical pointer.
 * 
 * @param[in] p_ptr The pointer in linear memory space.
 * 
 * @returns The pointer in physical memory space.
 */
static inline void *pmm_linearToPhysical(void *p_ptr) {
    return (void *)((uintptr_t)p_ptr - (uintptr_t)g_pmm_hhdm);
}

/**
 * @brief Converts a physical pointer to a linear pointer.
 * 
 * @param[in] p_ptr The pointer in physical memory space.
 * 
 * @returns The pointer in linear memory space.
 */
static inline void *pmm_physicalToLinear(void *p_ptr) {
    return (void *)((uintptr_t)p_ptr + (uintptr_t)g_pmm_hhdm);
}

#endif
