#include <stdbool.h>

#include "arch/amd64/asm.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "printk.h"
#include "string.h"

#define C_VMM_ENTRIES_PER_PAGE \
    (C_MM_PAGE_SIZE / sizeof(struct ts_mm_memoryMapEntryListNode))

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
static struct ts_mm_memoryMapEntryListNode *vmmAllocateEntryPoolEntry(
    struct ts_vmm_context *p_context
);
static void vmm_freeNode(
    void *p_context,
    struct ts_mm_memoryMapEntryListNode *p_node
);
static int vmm_ensurePoolNotEmpty(struct ts_vmm_context *p_context);
static int vmm_tryFreePagingStructure(uint64_t *l_pagingStructure);

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

    return mm_alloc(&l_context->m_map, p_size);
}

int vmm_free(struct ts_vmm_context *p_context, void *p_ptr, size_t p_size) {
    size_t l_size = mm_roundUpPage(p_size);

    if(vmm_ensurePoolNotEmpty(p_context) != 0) {
        return -1;
    }

    struct ts_mm_memoryMapEntryListNode *l_newNode = p_context->m_mapEntryPool;

    p_context->m_mapEntryPool = l_newNode->m_next;

    l_newNode->m_data.m_base = p_ptr;
    l_newNode->m_data.m_size = l_size;

    mm_addNodeToMap(&p_context->m_map, l_newNode, vmm_freeNode, p_context);

    return 0;
}

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
    size_t l_pml4Index = ((uintptr_t)p_vptr >> 39UL) & 0x1ffUL;
    size_t l_pdptIndex = ((uintptr_t)p_vptr >> 30UL) & 0x1ffUL;
    size_t l_pdIndex = ((uintptr_t)p_vptr >> 21UL) & 0x1ffUL;
    size_t l_ptIndex = ((uintptr_t)p_vptr >> 12UL) & 0x1ffUL;
    uintptr_t l_currentPhysicalAddress =
        (uintptr_t)p_pptr & 0x000ffffffffff000UL;
    uint64_t *l_pml4 = (uint64_t *)p_context->m_pagingContext;

    uint64_t l_flags = 0;

    if((p_flags & C_VMM_PROT_USER) != 0UL) {
        l_flags |= C_VMM_PAGING_FLAG_USER_SUPERVISOR;
    }

    if((p_flags & C_VMM_PROT_READ_WRITE) != 0UL) {
        l_flags |= C_VMM_PAGING_FLAG_READ_WRITE;
    }

    while(l_pml4Index < 512 && l_size != 0) {
        if((l_pml4[l_pml4Index] & C_VMM_PAGING_FLAG_PRESENT) == 0UL) {
            uint64_t *l_pdpt = pmm_alloc(C_MM_PAGE_SIZE);

            if(l_pdpt == NULL) {
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
    uint64_t *l_pml4 = (uint64_t *)p_context->m_pagingContext;

    while(l_pml4Index < 512 && l_size != 0) {
        if((l_pml4[l_pml4Index] & C_VMM_PAGING_FLAG_PRESENT) == 0UL) {
            return -1;
        }

        uint64_t *l_pdpt =
            (uint64_t *)(l_pml4[l_pml4Index] & 0x000ffffffffff000UL);

        while(l_pdptIndex < 512 && l_size != 0) {
            if((l_pdpt[l_pdptIndex] & C_VMM_PAGING_FLAG_PRESENT) == 0UL) {
                return -1;
            }

            uint64_t *l_pd =
                (uint64_t *)(l_pdpt[l_pdptIndex] & 0x000ffffffffff000UL);

            while(l_pdIndex < 512 && l_size != 0) {
                if((l_pd[l_pdIndex] & C_VMM_PAGING_FLAG_PRESENT) == 0UL) {
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
    uint64_t *l_pml4 = (uint64_t *)p_context->m_pagingContext;
    int l_pml4Index = ((uintptr_t)p_vptr >> 39UL) & 0x1ff;

    if((l_pml4[l_pml4Index] & C_VMM_PAGING_FLAG_PRESENT) == 0) {
        return NULL;
    }

    uint64_t *l_pdpt = (uint64_t *)(l_pml4[l_pml4Index] & 0x000ffffffffff000);
    int l_pdptIndex = ((uintptr_t)p_vptr >> 30UL) & 0x1ff;
    uintptr_t l_pdptAddress = l_pdpt[l_pdptIndex] & 0x000ffffffffff000;

    if((l_pdpt[l_pdptIndex] & C_VMM_PAGING_FLAG_PRESENT) == 0) {
        return NULL;
    }

    if((l_pdpt[l_pdptIndex] & C_VMM_PAGING_FLAG_PS) != 0) {
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
        return NULL;
    }

    if((l_pageDirectory[l_pageDirectoryIndex] & C_VMM_PAGING_FLAG_PS) != 0) {
        return (void *)(l_pageDirectoryAddress & 0x000fffffffe00000);
    }

    uint64_t *l_pageTable = (uint64_t *)l_pageDirectoryAddress;
    int l_pageTableIndex = ((uintptr_t)p_vptr >> 12UL) & 0x1ff;
    uintptr_t l_pageTableAddress =
        l_pageTable[l_pageTableIndex] & 0x000ffffffffff000;

    if((l_pageTable[l_pageTableIndex] & C_VMM_PAGING_FLAG_PRESENT) == 0) {
        return NULL;
    }

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
    struct ts_mm_memoryMapEntryListNode *l_kernelSpaceEntry =
        vmmAllocateEntryPoolEntry(&s_vmmKernelContext);

    if(l_kernelSpaceEntry == NULL) {
        return -1;
    }

    l_kernelSpaceEntry->m_next = NULL;
    l_kernelSpaceEntry->m_data.m_base = (void *)0xffffff8000000000;
    l_kernelSpaceEntry->m_data.m_size = 0x0000008000000000;

    mm_addNodeToMap(
        &s_vmmKernelContext.m_map,
        l_kernelSpaceEntry,
        vmm_freeNode,
        NULL
    );

    return 0;
}

static int vmm_refillContextEntryPool(struct ts_vmm_context *p_context) {
    struct ts_mm_memoryMapEntryListNode *l_nodePool = pmm_alloc(C_MM_PAGE_SIZE);

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

static struct ts_mm_memoryMapEntryListNode *vmmAllocateEntryPoolEntry(
    struct ts_vmm_context *p_context
) {
    if(vmm_ensurePoolNotEmpty(p_context) != 0) {
        return NULL;
    }

    struct ts_mm_memoryMapEntryListNode *l_entry = p_context->m_mapEntryPool;

    p_context->m_mapEntryPool = l_entry->m_next;

    return l_entry;
}

static void vmm_freeNode(
    void *p_context,
    struct ts_mm_memoryMapEntryListNode *p_node
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
