#include <stdbool.h>

#include "arch/amd64/asm.h"
#include "errno.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "printk.h"
#include "stdlib.h"
#include "string.h"

#define C_VMM_ENTRIES_PER_PAGE \
    (C_MM_PAGE_SIZE / sizeof(struct ts_memoryRange_listNode))

#define C_VMM_PAGING_FLAG_PS (1 << 7)
#define C_VMM_PAGING_FLAG_ACCESSED (1 << 5)
#define C_VMM_PAGING_FLAG_CACHE_DISABLE (1 << 4)
#define C_VMM_PAGING_FLAG_WRITE_THROUGH (1 << 3)
#define C_VMM_PAGING_FLAG_USER_SUPERVISOR (1 << 2)
#define C_VMM_PAGING_FLAG_READ_WRITE (1 << 1)
#define C_VMM_PAGING_FLAG_PRESENT (1 << 0)

extern int g_kernelStart;
extern int g_kernelEnd;

static struct ts_vmm_context s_vmmKernelContext;

static int vmm_initKernelContext(void);
static int vmm_refillContextEntryPool(struct ts_vmm_context *p_context);
static struct ts_memoryRange_listNode *vmmAllocateEntryPoolEntry(
    struct ts_vmm_context *p_context
);
static void vmm_freeNode(
    void *p_context,
    struct ts_memoryRange_listNode *p_node
);
static int vmm_ensurePoolNotEmpty(struct ts_vmm_context *p_context);
static int vmm_tryFreePagingStructure(uint64_t *l_pagingStructure);
static void vmm_destroyPml4(uint64_t *p_pml4);
static void vmm_destroyPdpt(uint64_t *p_pdpt);
static void vmm_destroyPd(uint64_t *p_pd);

int vmm_init(void) {
    // 1. Create the kernel VMM context
    if(vmm_initKernelContext()) {
        return -1;
    }

    // 2. Identity map the first 4 GiB of memory
    if(
        vmm_map(
            &s_vmmKernelContext,
            (void *)0x0000000000001000,
            (void *)0x0000000000001000,
            (1UL << 32UL) - 0x1000UL,
            C_VMM_PROT_KERNEL | C_VMM_PROT_READ_WRITE
        ) != 0
    ) {
        return -1;
    }

    // 3. Map kernel to 0xffffffff80000000
    // 3.1. Get the physical address of the kernel from the current paging
    // structure
    struct ts_vmm_context l_temporaryKernelContext = {
        .m_pagingContext = asm_readCr3()
    };

    void *l_kernelPhysicalAddress =
        vmm_getPhysicalAddress2(
            &l_temporaryKernelContext,
            (void *)0xffffffff80000000
        );

    // 3.2. Map the kernel
    if(
        vmm_map(
            &s_vmmKernelContext,
            (void *)0xffffffff80000000,
            l_kernelPhysicalAddress,
            (uintptr_t)&g_kernelEnd - (uintptr_t)&g_kernelStart,
            C_VMM_PROT_KERNEL | C_VMM_PROT_READ_WRITE | C_VMM_PROT_EXEC
        ) != 0
    ) {
        return -1;
    }

    // 4. Reload CR3
    asm_writeCr3(s_vmmKernelContext.m_pagingContext);

    return 0;
}

void *vmm_alloc(struct ts_vmm_context *p_context, size_t p_size, int p_flags) {
    struct ts_vmm_context *l_context;

    if((p_flags & C_VMM_ALLOC_FLAG_KERNEL) != 0) {
        l_context = &s_vmmKernelContext;
    } else {
        l_context = p_context;
    }

    spinlock_acquire(&p_context->m_spinlock);
    
    void *l_returnValue = mm_allocPages(&l_context->m_map, p_size);

    spinlock_release(&l_context->m_spinlock);

    return l_returnValue;
}

int vmm_free(struct ts_vmm_context *p_context, void *p_ptr, size_t p_size) {
    size_t l_size = mm_roundUpPage(p_size);

    spinlock_acquire(&p_context->m_spinlock);

    if(vmm_ensurePoolNotEmpty(p_context) != 0) {
        spinlock_release(&p_context->m_spinlock);
        return -1;
    }

    struct ts_memoryRange_listNode *l_newNode = p_context->m_mapEntryPool;

    p_context->m_mapEntryPool = l_newNode->m_next;

    l_newNode->m_memoryRange.m_ptr = p_ptr;
    l_newNode->m_memoryRange.m_size = l_size;

    mm_addNodeToMap(&p_context->m_map, l_newNode, vmm_freeNode, p_context);

    spinlock_release(&p_context->m_spinlock);

    return 0;
}

// TODO: Fix leaking memory when allocation fails.
int vmm_map(
    struct ts_vmm_context *p_context,
    void *p_vptr,
    void *p_pptr,
    size_t p_size,
    int p_flags
) {
    // 1. Check addresses
    uintptr_t l_pageVirtualOffsetStart =
        (uintptr_t)p_vptr & 0xfffffffffffff000UL;

    size_t l_size = mm_roundUpPage(
        p_size + (uintptr_t)p_vptr - l_pageVirtualOffsetStart
    );

    // 2. Build the paging structures
    uint64_t l_flags = 0;

    if((p_flags & C_VMM_PROT_USER) != 0UL) {
        l_flags |= C_VMM_PAGING_FLAG_USER_SUPERVISOR;
    }

    if((p_flags & C_VMM_PROT_READ_WRITE) != 0UL) {
        l_flags |= C_VMM_PAGING_FLAG_READ_WRITE;
    }

    size_t l_pml4Index = ((uintptr_t)p_vptr >> 39UL) & 0x1ffUL;
    size_t l_pdptIndex = ((uintptr_t)p_vptr >> 30UL) & 0x1ffUL;
    size_t l_pdIndex = ((uintptr_t)p_vptr >> 21UL) & 0x1ffUL;
    size_t l_ptIndex = ((uintptr_t)p_vptr >> 12UL) & 0x1ffUL;
    uintptr_t l_currentPhysicalAddress =
        (uintptr_t)p_pptr & 0x000ffffffffff000UL;

    spinlock_acquire(&p_context->m_spinlock);

    uint64_t *l_pml4 = (uint64_t *)p_context->m_pagingContext;

    while(l_pml4Index < 512 && l_size != 0) {
        if((l_pml4[l_pml4Index] & C_VMM_PAGING_FLAG_PRESENT) == 0UL) {
            uint64_t *l_pdpt = pmm_alloc(C_MM_PAGE_SIZE);

            if(l_pdpt == NULL) {
                spinlock_release(&p_context->m_spinlock);
                return -1;
            }

            l_pml4[l_pml4Index] = (uint64_t)l_pdpt
                | C_VMM_PAGING_FLAG_USER_SUPERVISOR
                | C_VMM_PAGING_FLAG_READ_WRITE
                | C_VMM_PAGING_FLAG_PRESENT;
        }

        uint64_t *l_pdpt =
            (uint64_t *)(l_pml4[l_pml4Index] & 0x000ffffffffff000UL);

        while(l_pdptIndex < 512 && l_size != 0) {
            if((l_pdpt[l_pdptIndex] & C_VMM_PAGING_FLAG_PRESENT) == 0UL) {
                uint64_t *l_pd = pmm_alloc(C_MM_PAGE_SIZE);

                if(l_pd == NULL) {
                    spinlock_release(&p_context->m_spinlock);
                    return -1;
                }

                l_pdpt[l_pdptIndex] = (uint64_t)l_pd
                    | C_VMM_PAGING_FLAG_USER_SUPERVISOR
                    | C_VMM_PAGING_FLAG_READ_WRITE
                    | C_VMM_PAGING_FLAG_PRESENT;
            }

            uint64_t *l_pd =
                (uint64_t *)(l_pdpt[l_pdptIndex] & 0x000ffffffffff000UL);

            while(l_pdIndex < 512 && l_size != 0) {
                if((l_pd[l_pdIndex] & C_VMM_PAGING_FLAG_PRESENT) == 0UL) {
                    uint64_t *l_pt = pmm_alloc(C_MM_PAGE_SIZE);

                    if(l_pt == NULL) {
                        spinlock_release(&p_context->m_spinlock);
                        return -1;
                    }

                    l_pd[l_pdIndex] = (uint64_t)l_pt
                        | C_VMM_PAGING_FLAG_USER_SUPERVISOR
                        | C_VMM_PAGING_FLAG_READ_WRITE
                        | C_VMM_PAGING_FLAG_PRESENT;
                }

                uint64_t *l_pt =
                    (uint64_t *)(l_pd[l_pdIndex] & 0x000ffffffffff000UL);

                while(l_ptIndex < 512 && l_size != 0) {
                    l_pt[l_ptIndex] = l_currentPhysicalAddress
                        | l_flags
                        | C_VMM_PAGING_FLAG_PRESENT;

                    l_currentPhysicalAddress += 1UL << 12UL;
                    l_size -= 1UL << 12UL;
                    l_ptIndex++;
                }

                l_pdIndex++;
                l_ptIndex = 0;
            }

            l_pdptIndex++;
            l_pdIndex = 0;
        }

        l_pml4Index++;
        l_pdptIndex = 0;
    }

    spinlock_release(&p_context->m_spinlock);

    return 0;
}

int vmm_unmap(
    struct ts_vmm_context *p_context,
    void *p_ptr,
    size_t p_size
) {
    // 1. Check addresses
    uintptr_t l_pageVirtualOffsetStart =
        (uintptr_t)p_ptr & 0xfffffffffffff000UL;

    size_t l_size = mm_roundUpPage(
        p_size + (uintptr_t)p_ptr - l_pageVirtualOffsetStart
    );

    // 2. Build the paging structures
    size_t l_pml4Index = ((uintptr_t)p_ptr >> 39UL) & 0x1ffUL;
    size_t l_pdptIndex = ((uintptr_t)p_ptr >> 30UL) & 0x1ffUL;
    size_t l_pdIndex = ((uintptr_t)p_ptr >> 21UL) & 0x1ffUL;
    size_t l_ptIndex = ((uintptr_t)p_ptr >> 12UL) & 0x1ffUL;

    spinlock_acquire(&p_context->m_spinlock);

    uint64_t *l_pml4 = (uint64_t *)p_context->m_pagingContext;

    while(l_pml4Index < 512 && l_size != 0) {
        if((l_pml4[l_pml4Index] & C_VMM_PAGING_FLAG_PRESENT) == 0UL) {
            spinlock_release(&p_context->m_spinlock);
            return -1;
        }

        uint64_t *l_pdpt =
            (uint64_t *)(l_pml4[l_pml4Index] & 0x000ffffffffff000UL);

        while(l_pdptIndex < 512 && l_size != 0) {
            if((l_pdpt[l_pdptIndex] & C_VMM_PAGING_FLAG_PRESENT) == 0UL) {
                spinlock_release(&p_context->m_spinlock);
                return -1;
            }

            uint64_t *l_pd =
                (uint64_t *)(l_pdpt[l_pdptIndex] & 0x000ffffffffff000UL);

            while(l_pdIndex < 512 && l_size != 0) {
                if((l_pd[l_pdIndex] & C_VMM_PAGING_FLAG_PRESENT) == 0UL) {
                    spinlock_release(&p_context->m_spinlock);
                    return -1;
                }

                uint64_t *l_pt =
                    (uint64_t *)(l_pd[l_pdIndex] & 0x000ffffffffff000UL);

                while(l_ptIndex < 512 && l_size != 0) {
                    l_pt[l_ptIndex] = 0UL;
                    l_ptIndex++;
                    l_size -= 0x1000UL;
                }

                l_pdIndex++;
                l_ptIndex = 0;

                vmm_tryFreePagingStructure(l_pt);
            }

            l_pdptIndex++;
            l_pdIndex = 0;

            vmm_tryFreePagingStructure(l_pd);
        }

        l_pml4Index++;
        l_pdptIndex = 0;

        vmm_tryFreePagingStructure(l_pdpt);
    }
    
    spinlock_release(&p_context->m_spinlock);

    return 0;
}

struct ts_vmm_context *vmm_getKernelContext(void) {
    return &s_vmmKernelContext;
}

void *vmm_getPhysicalAddress(void *p_vptr) {
    struct ts_vmm_context l_context = {
        .m_pagingContext = asm_readCr3()
    };

    return vmm_getPhysicalAddress2(&l_context, p_vptr);
}

void *vmm_getPhysicalAddress2(
    struct ts_vmm_context *p_context,
    void *p_vptr
) {
    spinlock_acquire(&p_context->m_spinlock);

    uint64_t *l_pml4 = (uint64_t *)p_context->m_pagingContext;
    int l_pml4Index = ((uintptr_t)p_vptr >> 39UL) & 0x1ff;

    if((l_pml4[l_pml4Index] & C_VMM_PAGING_FLAG_PRESENT) == 0) {
        spinlock_release(&p_context->m_spinlock);
        return NULL;
    }

    uint64_t *l_pdpt = (uint64_t *)(l_pml4[l_pml4Index] & 0x000ffffffffff000);
    int l_pdptIndex = ((uintptr_t)p_vptr >> 30UL) & 0x1ff;
    uintptr_t l_pdptAddress = l_pdpt[l_pdptIndex] & 0x000ffffffffff000;

    if((l_pdpt[l_pdptIndex] & C_VMM_PAGING_FLAG_PRESENT) == 0) {
        spinlock_release(&p_context->m_spinlock);
        return NULL;
    }

    if((l_pdpt[l_pdptIndex] & C_VMM_PAGING_FLAG_PS) != 0) {
        spinlock_release(&p_context->m_spinlock);
        return (void *)(l_pdptAddress & 0x000fffffc0000000);
    }

    uint64_t *l_pageDirectory = (uint64_t *)l_pdptAddress;
    int l_pageDirectoryIndex = ((uintptr_t)p_vptr >> 21UL) & 0x1ff;
    uintptr_t l_pageDirectoryAddress =
        l_pageDirectory[l_pageDirectoryIndex] & 0x000ffffffffff000;

    if(
        (l_pageDirectory[l_pageDirectoryIndex] & C_VMM_PAGING_FLAG_PRESENT)
        == 0
    ) {
        spinlock_release(&p_context->m_spinlock);
        return NULL;
    }

    if((l_pageDirectory[l_pageDirectoryIndex] & C_VMM_PAGING_FLAG_PS) != 0) {
        spinlock_release(&p_context->m_spinlock);
        return (void *)(l_pageDirectoryAddress & 0x000fffffffe00000);
    }

    uint64_t *l_pageTable = (uint64_t *)l_pageDirectoryAddress;
    int l_pageTableIndex = ((uintptr_t)p_vptr >> 12UL) & 0x1ff;
    uintptr_t l_pageTableAddress =
        l_pageTable[l_pageTableIndex] & 0x000ffffffffff000;

    if((l_pageTable[l_pageTableIndex] & C_VMM_PAGING_FLAG_PRESENT) == 0) {
        spinlock_release(&p_context->m_spinlock);
        return NULL;
    }

    spinlock_release(&p_context->m_spinlock);

    return (void *)l_pageTableAddress;
}

static int vmm_initKernelContext(void) {
    s_vmmKernelContext.m_mapEntryPool = NULL;

    // Allocate a PML4 for the kernel.
    s_vmmKernelContext.m_pagingContext = (uintptr_t)pmm_alloc(C_MM_PAGE_SIZE);

    if(s_vmmKernelContext.m_pagingContext == 0) {
        return -1;
    }

    memset((void *)s_vmmKernelContext.m_pagingContext, 0, C_MM_PAGE_SIZE);

    // Allocate one entry from the pool to map usable kernel space between
    // [0xffff800000000000-0xffffffff7fffffff] (last PML4 entry - last 2 GiB)
    // The last 2 GiB are reserved for the kernel.
    struct ts_memoryRange_listNode *l_kernelSpaceEntry =
        vmmAllocateEntryPoolEntry(&s_vmmKernelContext);

    if(l_kernelSpaceEntry == NULL) {
        return -1;
    }

    l_kernelSpaceEntry->m_next = NULL;
    l_kernelSpaceEntry->m_memoryRange.m_ptr = (void *)0xffffff8000000000;
    l_kernelSpaceEntry->m_memoryRange.m_size = 0x0000008000000000;

    mm_addNodeToMap(
        &s_vmmKernelContext.m_map,
        l_kernelSpaceEntry,
        vmm_freeNode,
        NULL
    );

    spinlock_init(&s_vmmKernelContext.m_spinlock);

    return 0;
}

static int vmm_refillContextEntryPool(struct ts_vmm_context *p_context) {
    struct ts_memoryRange_listNode *l_nodePool = pmm_alloc(C_MM_PAGE_SIZE);

    if(l_nodePool == NULL) {
        return -1;
    }

    // Prepare entries [0, C_VMM_ENTRIES_PER_PAGE - 1]
    for(int l_index = 0; l_index < (int)C_VMM_ENTRIES_PER_PAGE - 1; l_index++) {
        l_nodePool[l_index].m_next = &l_nodePool[l_index + 1];
    }

    // Last entry points to the current entry pool
    l_nodePool[C_VMM_ENTRIES_PER_PAGE - 1].m_next = p_context->m_mapEntryPool;
    p_context->m_mapEntryPool = &l_nodePool[0];

    return 0;
}

static struct ts_memoryRange_listNode *vmmAllocateEntryPoolEntry(
    struct ts_vmm_context *p_context
) {
    if(vmm_ensurePoolNotEmpty(p_context) != 0) {
        return NULL;
    }

    struct ts_memoryRange_listNode *l_entry = p_context->m_mapEntryPool;

    p_context->m_mapEntryPool = l_entry->m_next;

    return l_entry;
}

static void vmm_freeNode(
    void *p_context,
    struct ts_memoryRange_listNode *p_node
) {
    struct ts_vmm_context *l_context = (struct ts_vmm_context *)p_context;

    p_node->m_next = l_context->m_mapEntryPool;
    l_context->m_mapEntryPool = p_node;
}

static int vmm_ensurePoolNotEmpty(struct ts_vmm_context *p_context) {
    if(p_context->m_mapEntryPool == NULL) {
        return vmm_refillContextEntryPool(p_context);
    } else {
        return 0;
    }
}

static int vmm_tryFreePagingStructure(uint64_t *l_pagingStructure) {
    bool l_allFree = true;
    int l_index = 0;

    while(l_allFree && l_index < 512) {
        l_allFree &=
            (l_pagingStructure[l_index] & C_VMM_PAGING_FLAG_PRESENT) != 0;
        l_index++;
    }

    if(!l_allFree) {
        return 0;
    }

    pmm_free(l_pagingStructure, C_MM_PAGE_SIZE);

    return 0;
}

struct ts_vmm_context *vmm_createContext(void) {
    // Allocate context
    struct ts_vmm_context *l_context = pmm_alloc(C_MM_PAGE_SIZE);

    if(l_context == NULL) {
        return NULL;
    }

    // Allocate a new PML4
    uint64_t *l_pml4 = pmm_alloc(C_MM_PAGE_SIZE);

    if(l_pml4 == NULL) {
        pmm_free(l_context, C_MM_PAGE_SIZE);
        return NULL;
    }

    l_context->m_map = NULL;
    l_context->m_mapEntryPool = NULL;
    l_context->m_pagingContext = (uintptr_t)l_pml4;
    spinlock_init(&l_context->m_spinlock);

    // Nothing is mapped initially
    memset(l_pml4, 0, C_MM_PAGE_SIZE - 8UL);

    // Copy the kernel entry from the kernel context
    memcpy(
        &l_pml4[511],
        (void *)(s_vmmKernelContext.m_pagingContext + 4088UL),
        8UL
    );

    return l_context;
}

void vmm_destroyContext(struct ts_vmm_context *p_context) {
    vmm_destroyPml4((uint64_t *)p_context->m_pagingContext);

    // TODO: map entries from entry pool are not freed!

    pmm_free(p_context, C_MM_PAGE_SIZE);
}

static void vmm_destroyPml4(uint64_t *p_pml4) {
    for(int l_indexPml4 = 0; l_indexPml4 < 511; l_indexPml4++) {
        if((p_pml4[l_indexPml4] & C_VMM_PAGING_FLAG_PRESENT) != 0) {
            vmm_destroyPdpt(
                (uint64_t *)(p_pml4[l_indexPml4] & 0xfffffffffffff000)
            );
        }
    }

    pmm_free(p_pml4, C_MM_PAGE_SIZE);
}

static void vmm_destroyPdpt(uint64_t *p_pdpt) {
    for(int l_indexPdpt = 0; l_indexPdpt < 512; l_indexPdpt++) {
        if((p_pdpt[l_indexPdpt] & C_VMM_PAGING_FLAG_PRESENT) != 0) {
            vmm_destroyPd(
                (uint64_t *)(p_pdpt[l_indexPdpt] & 0xfffffffffffff000)
            );
        }
    }

    pmm_free(p_pdpt, C_MM_PAGE_SIZE);
}

static void vmm_destroyPd(uint64_t *p_pd) {
    for(int l_indexPd = 0; l_indexPd < 512; l_indexPd++) {
        if((p_pd[l_indexPd] & C_VMM_PAGING_FLAG_PRESENT) != 0) {
            pmm_free(
                (void *)(p_pd[l_indexPd] & 0xfffffffffffff000),
                C_MM_PAGE_SIZE
            );
        }
    }

    pmm_free(p_pd, C_MM_PAGE_SIZE);
}
