#ifndef __INCLUDE_MM_VMM_H__
#define __INCLUDE_MM_VMM_H__

#include <stdint.h>

#include "mm/mm.h"

#define C_VMM_PROT_USER (1 << 0)
#define C_VMM_PROT_KERNEL 0
#define C_VMM_PROT_READ_ONLY 0
#define C_VMM_PROT_READ_WRITE (1 << 1)
#define C_VMM_PROT_DATA 0
#define C_VMM_PROT_EXEC (1 << 2)

#define C_VMM_ALLOC_FLAG_KERNEL (1 << 0)

struct ts_vmm_context {
    struct ts_mm_memoryMapEntryListNode *m_map;
    uintptr_t m_pagingContext;
    struct ts_mm_memoryMapEntryListNode *m_mapEntryPool;
};

/**
 * @brief Initializes the VMM.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success.
*/
int vmm_init(void);

/**
 * @brief Allocates some space in the virtual memory space.
 * 
 * @param[in] p_context The VMM context.
 * @param[in] p_size The number of bytes to allocate.
 * @param[in] p_flags The allocation flags.
 * 
 * @returns The pointer to the allocated zone.
 * @retval NULL if the allocation operation failed.
*/
void *vmm_alloc(struct ts_vmm_context *p_context, size_t p_size, int p_flags);

/**
 * @brief Frees some space in the virtual memory space.
 * 
 * @param[in] p_context The VMM context.
 * @param[in] p_ptr The pointer to the zone to free.
 * @param[in] p_size The number of bytes to free.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success.
 * 
 * @note Freeing a part of a previously allocated zone is undefined behavior.
*/
int vmm_free(struct ts_vmm_context *p_context, void *p_ptr, size_t p_size);

/**
 * @brief Maps a physical memory zone to a virtual memory zone.
 * 
 * @param[in] p_context The VMM context.
 * @param[in] p_vptr The address in virtual memory where the zone will be
 * mapped.
 * @param[in] p_pptr The address in physical memory of the zone that will be
 * mapped.
 * @param[in] p_size The size of the zone that will be mapped.
 * @param[in] p_flags The flags of the mapping operation.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success.
*/
int vmm_map(
    struct ts_vmm_context *p_context,
    void *p_vptr,
    void *p_pptr,
    size_t p_size,
    int p_flags
);

/**
 * @brief Maps a physical memory zone to a virtual memory zone.
 * 
 * @param[in] p_context The VMM context.
 * @param[in] p_ptr The address in virtual memory of the zone to unmap.
 * @param[in] p_size The size of the zone that will be unmapped.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success.
*/
int vmm_unmap(
    struct ts_vmm_context *p_context,
    void *p_ptr,
    size_t p_size
);

/**
 * @brief Returns the kernel VMM context.
 * 
 * @returns The kernel VMM context.
*/
struct ts_vmm_context *vmm_getKernelContext(void);

void *vmm_getPhysicalAddress(void *p_vptr);
void *vmm_getPhysicalAddress2(
    struct ts_vmm_context *p_context,
    void *p_vptr
);
struct ts_vmm_context *vmm_createContext(void);
void vmm_destroyContext(struct ts_vmm_context *p_context);

#endif
