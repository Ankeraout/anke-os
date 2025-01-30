#include <stdbool.h>

#include "arch/amd64/asm.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "string.h"

#define C_VMM_ENTRIES_PER_PAGE \
    (C_MM_PAGE_SIZE / sizeof(struct ts_mmMemoryMapEntryListNode))

#define C_VMM_PAGING_FLAG_PS (1 << 7)
#define C_VMM_PAGING_FLAG_ACCESSED (1 << 5)
#define C_VMM_PAGING_FLAG_CACHE_DISABLE (1 << 4)
#define C_VMM_PAGING_FLAG_WRITE_THROUGH (1 << 3)
#define C_VMM_PAGING_FLAG_USER_SUPERVISOR (1 << 2)
#define C_VMM_PAGING_FLAG_READ_WRITE (1 << 1)
#define C_VMM_PAGING_FLAG_PRESENT (1 << 0)

extern int g_kernelStart;
extern int g_kernelEnd;

static struct ts_vmmContext s_vmmKernelContext;

static int vmmInitKernelContext(void);
static int vmmRefillContextEntryPool(struct ts_vmmContext *p_context);
static struct ts_mmMemoryMapEntryListNode *vmmAllocateEntryPoolEntry(
    struct ts_vmmContext *p_context
);
static void vmmFreeNode(
    void *p_context,
    struct ts_mmMemoryMapEntryListNode *p_node
);
static int vmmEnsurePoolNotEmpty(struct ts_vmmContext *p_context);
static int vmmTryFreePagingStructure(uint64_t *l_pagingStructure);

int vmmInit(void) {
    // 1. Create the kernel VMM context
    if(vmmInitKernelContext()) {
        return -1;
    }

    // 2. Identity map the first 512 GiB of memory
    // 2.1. Create the PDPT
    uint64_t *l_identityPdpt = pmmAlloc(C_MM_PAGE_SIZE);

    if(l_identityPdpt == NULL) {
        return -1;
    }

    // 2.2. Initialize the PDPT entries
    uint64_t l_entryValue = 0
        | C_VMM_PAGING_FLAG_PS
        | C_VMM_PAGING_FLAG_READ_WRITE
        | C_VMM_PAGING_FLAG_PRESENT;

    for(int l_index = 0; l_index < 512; l_index++) {
        l_identityPdpt[l_index] = l_entryValue;
        l_entryValue += 0x0000000040000000;
    }

    // 2.3. Set the PML4 entry
    ((uint64_t *)s_vmmKernelContext.m_pagingContext)[0] =
        (uint64_t)l_identityPdpt
        | C_VMM_PAGING_FLAG_READ_WRITE
        | C_VMM_PAGING_FLAG_PRESENT;

    // 3. Map kernel to 0xffffffff80000000
    // 3.1. Create the PDPT
    uint64_t *l_kernelPdpt = pmmAlloc(C_MM_PAGE_SIZE);

    if(l_kernelPdpt == NULL) {
        return -1;
    }

    // 3.2. Create the page directory
    uint64_t *l_kernelPageDirectory = pmmAlloc(C_MM_PAGE_SIZE);

    if(l_kernelPageDirectory == NULL) {
        return -1;
    }

    // 3.3. Initialize the PDPT entry
    l_kernelPdpt[0x1ff] = (uintptr_t)l_kernelPageDirectory
        | C_VMM_PAGING_FLAG_READ_WRITE
        | C_VMM_PAGING_FLAG_PRESENT;

    // 3.4. Get the physical address of the kernel from the current paging
    // structure
    struct ts_vmmContext l_temporaryKernelContext = {
        .m_pagingContext = readCr3()
    };

    void *l_kernelPhysicalAddress =
        vmmGetPhysicalAddress2(
            &l_temporaryKernelContext,
            (void *)0xffffffff80000000
        );

    // 3.5. Map the kernel
    if(
        vmmMap(
            &s_vmmKernelContext,
            (void *)0xffffffff80000000,
            l_kernelPhysicalAddress,
            (uintptr_t)&g_kernelEnd - (uintptr_t)&g_kernelStart,
            C_VMM_PROT_KERNEL | C_VMM_PROT_READ_WRITE | C_VMM_PROT_EXEC
        ) != 0
    ) {
        return -1;
    }

    // Reload CR3
    writeCr3(s_vmmKernelContext.m_pagingContext);

    return 0;
}

void *vmmAlloc(struct ts_vmmContext *p_context, size_t p_size, int p_flags) {
    struct ts_vmmContext *l_context;

    if((p_flags & C_VMM_ALLOC_FLAG_KERNEL) != 0) {
        l_context = &s_vmmKernelContext;
    } else {
        l_context = p_context;
    }

    return mmAlloc(&l_context->m_map, p_size);
}

int vmmFree(struct ts_vmmContext *p_context, void *p_ptr, size_t p_size) {
    size_t l_size = mmRoundUpPage(p_size);

    if(vmmEnsurePoolNotEmpty(p_context) != 0) {
        return -1;
    }

    struct ts_mmMemoryMapEntryListNode *l_newNode = p_context->m_mapEntryPool;

    p_context->m_mapEntryPool = l_newNode->m_next;

    l_newNode->m_data.m_base = p_ptr;
    l_newNode->m_data.m_size = l_size;

    mmAddNodeToMap(&p_context->m_map, l_newNode, vmmFreeNode, p_context);

    return 0;
}

int vmmMap(
    struct ts_vmmContext *p_context,
    void *p_vptr,
    void *p_pptr,
    size_t p_size,
    int p_flags
) {
    // 1. Check addresses
    uintptr_t l_pageVirtualOffsetStart =
        (uintptr_t)p_vptr & 0xfffffffffffff000UL;

    size_t l_size = mmRoundUpPage(
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
            uint64_t *l_pdpt = pmmAlloc(C_MM_PAGE_SIZE);

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
                uint64_t *l_pd = pmmAlloc(C_MM_PAGE_SIZE);

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
                    uint64_t *l_pt = pmmAlloc(C_MM_PAGE_SIZE);

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
            }

            l_pdptIndex++;
        }

        l_pml4Index++;
    }

    return 0;
}

int vmmUnmap(
    struct ts_vmmContext *p_context,
    void *p_ptr,
    size_t p_size
) {
    // 1. Check addresses
    uintptr_t l_pageVirtualOffsetStart =
        (uintptr_t)p_ptr & 0xfffffffffffff000UL;

    size_t l_size = mmRoundUpPage(
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

                vmmTryFreePagingStructure(l_pt);
            }

            l_pdptIndex++;

            vmmTryFreePagingStructure(l_pd);
        }

        l_pml4Index++;

        vmmTryFreePagingStructure(l_pdpt);
    }

    return 0;
}

struct ts_vmmContext *vmmGetKernelContext(void) {
    return &s_vmmKernelContext;
}

void *vmmGetPhysicalAddress(void *p_vptr) {
    struct ts_vmmContext l_context = {
        .m_pagingContext = readCr3()
    };

    return vmmGetPhysicalAddress2(&l_context, p_vptr);
}

void *vmmGetPhysicalAddress2(
    struct ts_vmmContext *p_context,
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

static int vmmInitKernelContext(void) {
    s_vmmKernelContext.m_mapEntryPool = NULL;

    // Allocate a PML4 for the kernel.
    s_vmmKernelContext.m_pagingContext = (uintptr_t)pmmAlloc(C_MM_PAGE_SIZE);

    if(s_vmmKernelContext.m_pagingContext == 0) {
        return -1;
    }

    memset((void *)s_vmmKernelContext.m_pagingContext, 0, C_MM_PAGE_SIZE);

    // Allocate one entry from the pool to map usable kernel space between
    // [0xffff800000000000-0xffffffff7fffffff] (last PML4 entry - last 2 GiB)
    // The last 2 GiB are reserved for the kernel.
    struct ts_mmMemoryMapEntryListNode *l_kernelSpaceEntry =
        vmmAllocateEntryPoolEntry(&s_vmmKernelContext);

    if(l_kernelSpaceEntry == NULL) {
        return -1;
    }

    l_kernelSpaceEntry->m_next = NULL;
    l_kernelSpaceEntry->m_data.m_base = (void *)0xffffff8000000000;
    l_kernelSpaceEntry->m_data.m_size = 0x0000008000000000;

    mmAddNodeToMap(
        &s_vmmKernelContext.m_map,
        l_kernelSpaceEntry,
        vmmFreeNode,
        NULL
    );

    return 0;
}

static int vmmRefillContextEntryPool(struct ts_vmmContext *p_context) {
    struct ts_mmMemoryMapEntryListNode *l_nodePool = pmmAlloc(C_MM_PAGE_SIZE);

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

static struct ts_mmMemoryMapEntryListNode *vmmAllocateEntryPoolEntry(
    struct ts_vmmContext *p_context
) {
    if(vmmEnsurePoolNotEmpty(p_context) != 0) {
        return NULL;
    }

    struct ts_mmMemoryMapEntryListNode *l_entry = p_context->m_mapEntryPool;

    p_context->m_mapEntryPool = l_entry->m_next;

    return l_entry;
}

static void vmmFreeNode(
    void *p_context,
    struct ts_mmMemoryMapEntryListNode *p_node
) {
    struct ts_vmmContext *l_context = (struct ts_vmmContext *)p_context;

    p_node->m_next = l_context->m_mapEntryPool;
    l_context->m_mapEntryPool = p_node;
}

static int vmmEnsurePoolNotEmpty(struct ts_vmmContext *p_context) {
    if(p_context->m_mapEntryPool == NULL) {
        return vmmRefillContextEntryPool(p_context);
    } else {
        return 0;
    }
}

static int vmmTryFreePagingStructure(uint64_t *l_pagingStructure) {
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

    pmmFree(l_pagingStructure, C_MM_PAGE_SIZE);

    return 0;
}
