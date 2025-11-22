#ifndef __INCLUDE_MM_VMM_H__
#define __INCLUDE_MM_VMM_H__

#include <stdint.h>

#include "mm/mm.h"
#include "spinlock.h"

#define C_VMM_PROT_READ_ONLY 0
#define C_VMM_PROT_READ_WRITE (1 << 1)
#define C_VMM_PROT_EXEC (1 << 2)
#define C_VMM_PROT_RO 0
#define C_VMM_PROT_RW C_VMM_PROT_READ_WRITE
#define C_VMM_PROT_RX C_VMM_PROT_READ_WRITE | C_VMM_PROT_EXEC
#define C_VMM_PROT_X C_VMM_PROT_EXEC
#define C_VMM_CACHE_WRITEBACK 0
#define C_VMM_CACHE_WRITETHROUGH (1 << 3)
#define C_VMM_CACHE_UNCACHED_MTRR (2 << 3)
#define C_VMM_CACHE_UNCACHED (3 << 3)
#define C_VMM_CACHE_WRITEPROTECT (4 << 3)
#define C_VMM_CACHE_WRITECOMBINING (5 << 3)

struct ts_vmm_context {
    /**
     * @brief This list contains the memory ranges (pages) that were allocated.
     */
    struct ts_memoryRange_listNode *m_memoryAllocations;

    /**
     * @brief This list stores the free virtual memory space entries.
     */
    struct ts_memoryRange_listNode *m_map;

    /**
     * @brief This list stores the free memory range entries of the VMM context.
     */
    struct ts_memoryRange_listNode *m_mapEntryPool;

    /**
     * @brief This member contains the pointer to the physical address of the
     * PML4.
     */
    void *m_pagingContext;
    t_spinlock m_spinlock;
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
 * @note This operation does not unmap memory.
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
 * @param[in] p_flags The flags of the mapping operation (C_VMM_PROT_* and
 * C_VMM_CACHE_*).
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
 * 
 * @note This operation does not free the memory.
*/
int vmm_unmap(
    struct ts_vmm_context *p_context,
    void *p_ptr,
    size_t p_size
);

void *vmm_getPhysicalAddress(void *p_vptr);
void *vmm_getPhysicalAddress2(
    struct ts_vmm_context *p_context,
    void *p_vptr
);

/**
 * @brief Creates a new user VMM context that can be used for applications.
 * 
 * @returns A new user VMM context that can be used for applications.
 */
struct ts_vmm_context *vmm_createContext(void);

/**
 * @brief Destroys a user VMM context.
 * 
 * @param[in] p_context The VMM context to destroy.
 */
void vmm_destroyContext(struct ts_vmm_context *p_context);

/**
 * @brief This variable contains the kernel VMM context.
 */
extern struct ts_vmm_context g_vmm_kernelContext;

#endif
