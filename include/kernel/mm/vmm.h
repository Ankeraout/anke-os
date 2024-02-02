#ifndef __INCLUDE_KERNEL_MM_VMM_H__
#define __INCLUDE_KERNEL_MM_VMM_H__

#include <stdint.h>

#include "kernel/mm/mm.h"

#define C_VMM_PROT_USER (1 << 0)
#define C_VMM_PROT_KERNEL 0
#define C_VMM_PROT_READ_ONLY 0
#define C_VMM_PROT_READ_WRITE (1 << 1)
#define C_VMM_PROT_DATA 0
#define C_VMM_PROT_EXEC (1 << 2)

#define C_VMM_ALLOC_FLAG_KERNEL (1 << 0)

struct ts_vmmContext {
    struct ts_mmMemoryMapEntryListNode *m_map;
    uintptr_t m_pagingContext;
    struct ts_mmMemoryMapEntryListNode *m_mapEntryPool;
};

/**
 * @brief Initializes the VMM.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success.
*/
int vmmInit(void);

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
void *vmmAlloc(struct ts_vmmContext *p_context, size_t p_size, int p_flags);

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
int vmmFree(struct ts_vmmContext *p_context, void *p_ptr, size_t p_size);

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
int vmmMap(
    struct ts_vmmContext *p_context,
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
int vmmUnmap(
    struct ts_vmmContext *p_context,
    void *p_ptr,
    size_t p_size
);

/**
 * @brief Gets the physical address that corresponds to the given virtual
 * address.
 * 
 * @param[in] p_vptr The virtual address.
 * 
 * @returns The physical address that corresponds to the given virtual address.
*/
void *vmmGetPhysicalAddress(void *p_vptr);

/**
 * @brief Gets the physical address that corresponds to the given virtual
 * address.
 * 
 * @param[in] p_context The VMM context.
 * @param[in] p_vptr The virtual address.
 * 
 * @returns The physical address that corresponds to the given virtual address.
*/
void *vmmGetPhysicalAddress2(struct ts_vmmContext *p_context, void *p_vptr);

struct ts_vmmContext *vmmGetKernelContext(void);

#endif
