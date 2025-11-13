#include <stdbool.h>

#include "arch/amd64/asm.h"
#include "bootstrap.h"
#include "errno.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "panic.h"
#include "printk.h"
#include "stdlib.h"
#include "string.h"

#define C_VMM_ENTRIES_PER_PAGE \
    (C_MM_PAGE_SIZE / sizeof(struct ts_memoryRange_listNode))

#define C_VMM_PAGING_FLAG_PAT (1 << 7)
#define C_VMM_PAGING_FLAG_PAT_PS (1 << 12)
#define C_VMM_PAGING_FLAG_PS (1 << 7)
#define C_VMM_PAGING_FLAG_ACCESSED (1 << 5)
#define C_VMM_PAGING_FLAG_PCD (1 << 4)
#define C_VMM_PAGING_FLAG_PWT (1 << 3)
#define C_VMM_PAGING_FLAG_USER_SUPERVISOR (1 << 2)
#define C_VMM_PAGING_FLAG_READ_WRITE (1 << 1)
#define C_VMM_PAGING_FLAG_PRESENT (1 << 0)

static void vmm_refillEntryPool(struct ts_vmm_context *p_context);
static void vmm_refillEntryPoolIfNeeded(struct ts_vmm_context *p_context);
static void vmm_initPAT(void);
static void vmm_initKernelContext(void);
static void vmm_initHHDM(void);

extern int g_kernelStart;
extern int g_kernelEnd;

static const uint64_t s_vmm_maskPAT[8] = {
    0,
    C_VMM_PAGING_FLAG_PWT,
    C_VMM_PAGING_FLAG_PCD,
    C_VMM_PAGING_FLAG_PWT | C_VMM_PAGING_FLAG_PCD,
    C_VMM_PAGING_FLAG_PAT,
    C_VMM_PAGING_FLAG_PAT | C_VMM_PAGING_FLAG_PCD,
    C_VMM_PAGING_FLAG_PAT | C_VMM_PAGING_FLAG_PWT,
    C_VMM_PAGING_FLAG_PAT | C_VMM_PAGING_FLAG_PCD | C_VMM_PAGING_FLAG_PWT
};

static const uint64_t s_vmm_maskNonPAT[8] = {
    0,
    C_VMM_PAGING_FLAG_PWT,
    C_VMM_PAGING_FLAG_PCD,
    C_VMM_PAGING_FLAG_PWT | C_VMM_PAGING_FLAG_PCD,
    0,
    C_VMM_PAGING_FLAG_PWT,
    C_VMM_PAGING_FLAG_PCD,
    C_VMM_PAGING_FLAG_PWT | C_VMM_PAGING_FLAG_PCD,
};

struct ts_vmm_context g_vmm_kernelContext;

/**
 * @brief This pointer points to the list of masks to use for page table
 * entries.
 */
static const uint64_t *s_vmm_pteMask;

/**
 * @brief This list contains the free memory ranges in the MMIO area.
 */
static struct ts_memoryRange_listNode *s_vmm_mmioMap;

/**
 * @brief This variable contains the kernel PML4.
 */
static __attribute__((aligned(0x1000))) uint64_t s_vmm_kernelPml4[512];

int vmm_init(void) {
    vmm_initPAT();
    vmm_initKernelContext();
    vmm_initHHDM();
    //vmm_mapKernel();
    //vmm_switchToKernelContext();
    //vmm_reclaimBootloaderMemory();

    return 0;
}

void *vmm_alloc(struct ts_vmm_context *p_context, size_t p_size, int p_flags) {

}

int vmm_free(struct ts_vmm_context *p_context, void *p_ptr, size_t p_size) {

}

int vmm_map(
    struct ts_vmm_context *p_context,
    void *p_vptr,
    void *p_pptr,
    size_t p_size,
    int p_flags
) {

}

int vmm_unmap(
    struct ts_vmm_context *p_context,
    void *p_ptr,
    size_t p_size
) {

}

void *vmm_getPhysicalAddress(void *p_vptr) {
    struct ts_vmm_context l_context = {
        .m_pagingContext = (void *)asm_readCr3()
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

struct ts_vmm_context *vmm_createContext(void) {

}

void vmm_destroyContext(struct ts_vmm_context *p_context) {

}

void *vmm_mapMmio(void *p_pptr, size_t p_size, int p_flags) {

}

void vmm_unmapMmio(void *p_vptr, size_t p_size) {

}

static void vmm_refillEntryPool(struct ts_vmm_context *p_context) {
    // Allocate a new memory page to store the data
    void *l_newPage_physicalAddress = pmm_alloc(C_MM_PAGE_SIZE);

    if(l_newPage_physicalAddress == NULL) {
        panic("vmm_refillEntryPool(): pmm_alloc() failed.");
    }

    struct ts_memoryRange_listNode *l_newPage =
        pmm_physicalToLinear(l_newPage_physicalAddress);

    // Refill the entry pool
    for(unsigned int l_index = 1; l_index < C_VMM_ENTRIES_PER_PAGE; l_index++) {
        l_newPage->m_next = p_context->m_mapEntryPool;
        p_context->m_mapEntryPool = l_newPage;
    }

    // Create a new memory allocation entry in the context
    l_newPage[0].m_next = p_context->m_memoryAllocations;
    l_newPage[0].m_memoryRange.m_ptr = l_newPage_physicalAddress;
    l_newPage[0].m_memoryRange.m_size = C_MM_PAGE_SIZE;
}

static void vmm_refillEntryPoolIfNeeded(struct ts_vmm_context *p_context) {
    if(p_context->m_mapEntryPool == NULL) {
        vmm_refillEntryPool(p_context);
    }
}

static void vmm_initPAT(void) {
    uint32_t l_eax;
    uint32_t l_edx;

    // CPUID with EAX=1
    asm_cpuid(1, &l_eax, &l_edx);

    // If PAT is supported, then bit 16 of EDX will be set.
    if((l_edx & (1 << 16)) != 0) {
        // Initialize the IA32_PAT MSR
        asm_wrmsr(0x277, 0x0000010500070406);

        // Set the current page table entry mask to the PAT masks
        s_vmm_pteMask = s_vmm_maskPAT;
    } else {
        // Set the current page table entry mask to the non-PAT masks
        s_vmm_pteMask = s_vmm_maskNonPAT;
    }
}

static void vmm_initKernelContext(void) {
    g_vmm_kernelContext.m_pagingContext = &s_vmm_kernelPml4;
    spinlock_init(&g_vmm_kernelContext.m_spinlock);
    g_vmm_kernelContext.m_memoryAllocations = NULL;
    g_vmm_kernelContext.m_mapEntryPool = NULL;
    g_vmm_kernelContext.m_map = NULL;

    vmm_refillEntryPool(&g_vmm_kernelContext);

    if(g_vmm_kernelContext.m_mapEntryPool == NULL) {
        panic("vmm: Failed to initialize kernel context.");
    }
}

static void vmm_initHHDM(void) {
    struct ts_memoryRange *l_memoryMap;
    size_t l_memoryMapEntryCount;

    bootstrap_getMemoryMap(&l_memoryMap, &l_memoryMapEntryCount);

    for(size_t l_index = 0UL; l_index < l_memoryMapEntryCount; l_index++) {
        const uintptr_t l_startAddress = (uintptr_t)l_memoryMap[l_index].m_ptr;
        const size_t l_size = l_memoryMap[l_index].m_size;
        const uintptr_t l_fixedStartAddress =
            (l_startAddress + 0xfffUL) & 0xfffffffffffff000UL;
        const size_t l_offset = l_fixedStartAddress - l_startAddress;

        if(l_offset >= l_size) {
            continue;
        }

        const size_t l_fixedSize = (l_size - l_offset) & 0xfffffffffffff000UL;
        
        if(l_fixedSize == 0UL) {
            continue;
        }

        size_t l_pageIndex = (l_startAddress + 0x0000fff000000000UL) >> 12UL;
        size_t l_pml4Index = l_pageIndex >> 27UL;
        size_t l_pdptIndex = (l_pageIndex >> 18UL) & 0x1ffUL;
        size_t l_pdIndex = (l_pageIndex >> 9UL) & 0x1ffUL;
        size_t l_ptIndex = l_pageIndex & 0x1ffUL;
        size_t l_pageCount = l_fixedSize >> 12UL;

        uint64_t *l_pdpt = pmm_physicalToLinear(
            (void *)(s_vmm_kernelPml4[l_pml4Index] & 0x000ffffffffff000UL)
        );
        uint64_t *l_pd = pmm_physicalToLinear(
            (void *)(l_pdpt[l_pdptIndex] & 0x000ffffffffff000UL)
        );
        uint64_t *l_pt = pmm_physicalToLinear(
            (void *)(l_pd[l_pdIndex] & 0x000ffffffffff000UL)
        );

        while(l_pageCount != 0UL) {


            l_ptIndex++;

            if(l_ptIndex == 512UL) {
                l_ptIndex = 0UL;
                l_pdIndex++;

                l_pt = pmm_physicalToLinear(
                    (void *)(l_pd[l_pdIndex] & 0x000ffffffffff000UL)
                );

                if(l_pdIndex == 512UL) {
                    l_pdIndex = 0UL;
                    l_pdptIndex++;

                    l_pd = pmm_physicalToLinear(
                        (void *)(l_pdpt[l_pdptIndex] & 0x000ffffffffff000UL)
                    );
                    l_pt = pmm_physicalToLinear(
                        (void *)(l_pd[l_pdIndex] & 0x000ffffffffff000UL)
                    );

                    if(l_pdptIndex == 512UL) {
                        l_pdptIndex = 0UL;
                        l_pml4Index++;

                        l_pdpt = pmm_physicalToLinear(
                            (void *)(s_vmm_kernelPml4[l_pml4Index] & 0x000ffffffffff000UL)
                        );
                        l_pd = pmm_physicalToLinear(
                            (void *)(l_pdpt[l_pdptIndex] & 0x000ffffffffff000UL)
                        );
                        l_pt = pmm_physicalToLinear(
                            (void *)(l_pd[l_pdIndex] & 0x000ffffffffff000UL)
                        );
                    }
                }
            }
        }
    }
}
