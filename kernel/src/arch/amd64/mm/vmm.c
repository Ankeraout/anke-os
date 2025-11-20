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

#define C_VMM_PAGING_FLAG_EXECUTE_DISABLE (1 << 63)
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
static void vmm_mapKernel(void);
static void vmm_switchToKernelContext(void);

extern int g_kernelStart;
extern int g_kernelEnd;
extern int g_kernelTextStart;
extern int g_kernelTextEnd;
extern int g_kernelRodataStart;
extern int g_kernelRodataEnd;
extern int g_kernelDataStart;
extern int g_kernelDataEnd;
extern int g_kernelBssStart;
extern int g_kernelBssEnd;

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
    vmm_mapKernel();
    vmm_switchToKernelContext();
    //vmm_reclaimBootloaderMemory();

    pr_info("vmm: Kernel context at 0x%016lx.\n", &g_vmm_kernelContext);

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
    // Page offsets must be the same
    size_t l_physicalOffset = (size_t)((uintptr_t)p_pptr) & 0xfffUL;
    size_t l_virtualOffset = (size_t)((uintptr_t)p_vptr) & 0xfffUL;

    if(l_physicalOffset != l_virtualOffset) {
        return EINVAL;
    }

    // Align virtual and physical memory ranges
    struct ts_memoryRange l_virtualRange = {
        .m_ptr = p_vptr,
        .m_size = p_size
    };
    struct ts_memoryRange l_physicalRange = {
        .m_ptr = p_pptr,
        .m_size = p_size
    };

    mm_alignRange(&l_virtualRange);
    mm_alignRange(&l_physicalRange);

    // Compute the number of pages
    size_t l_remainingPages = l_physicalRange.m_size >> 12UL;

    // Acquire lock on the context
    spinlock_acquire(&p_context->m_spinlock);

    // Map individual pages
    uint64_t *l_pml4 = pmm_physicalToLinear(p_context->m_pagingContext);
    uint64_t *l_pdpt = NULL;
    uint64_t *l_pd = NULL;
    uint64_t *l_pt = NULL;
    size_t l_pml4Index = (((uintptr_t)p_vptr) >> 39UL) & 0x1ffUL;
    size_t l_pdptIndex = (((uintptr_t)p_vptr) >> 30UL) & 0x1ffUL;
    size_t l_pdIndex = (((uintptr_t)p_vptr) >> 21UL) & 0x1ffUL;
    size_t l_ptIndex = (((uintptr_t)p_vptr) >> 12UL) & 0x1ffUL;
    size_t l_currentOffset = 0;
    bool error = false;
    const uint64_t l_pdFlags = C_VMM_PAGING_FLAG_PRESENT
        | C_VMM_PAGING_FLAG_READ_WRITE
        | C_VMM_PAGING_FLAG_USER_SUPERVISOR
        | C_VMM_PAGING_FLAG_PWT
        | C_VMM_PAGING_FLAG_ACCESSED;

    uint64_t l_ptFlags = C_VMM_PAGING_FLAG_PRESENT
        | C_VMM_PAGING_FLAG_ACCESSED
        | s_vmm_pteMask[(p_flags >> 3) & 0x07];

    if((p_flags & C_VMM_PROT_READ_WRITE) != 0) {
        l_ptFlags |= C_VMM_PAGING_FLAG_READ_WRITE;
    }

    if(l_pml4Index < 256) {
        l_ptFlags |= C_VMM_PAGING_FLAG_USER_SUPERVISOR;
    }

    while(l_remainingPages != 0UL) {
        if(l_pdpt == NULL) {
            const uint64_t l_pml4Entry = l_pml4[l_pml4Index];

            if((l_pml4[l_pml4Index] & C_VMM_PAGING_FLAG_PRESENT) != 0) {
                l_pdpt = pmm_physicalToLinear(
                    (void *)(l_pml4Entry & 0x000ffffffffff000UL)
                );
            } else {
                uint64_t *l_pdptPhysical = pmm_alloc(C_MM_PAGE_SIZE);

                if(l_pdptPhysical == NULL) {
                    error = true;
                    break;
                }

                l_pdpt = pmm_physicalToLinear(l_pdptPhysical);
                memset(l_pdpt, 0, C_MM_PAGE_SIZE);

                l_pml4[l_pml4Index] = (uint64_t)l_pdptPhysical | l_pdFlags;
            }
        }

        if(l_pd == NULL) {
            const uint64_t l_pdptEntry = l_pdpt[l_pdptIndex];

            if((l_pdpt[l_pdptIndex] & C_VMM_PAGING_FLAG_PRESENT) != 0) {
                l_pd = pmm_physicalToLinear(
                    (void *)(l_pdptEntry & 0x000ffffffffff000UL)
                );
            } else {
                uint64_t *l_pdPhysical = pmm_alloc(C_MM_PAGE_SIZE);

                if(l_pdPhysical == NULL) {
                    error = true;
                    break;
                }

                l_pd = pmm_physicalToLinear(l_pdPhysical);
                memset(l_pd, 0, C_MM_PAGE_SIZE);

                l_pdpt[l_pdptIndex] = (uint64_t)l_pdPhysical | l_pdFlags;
            }
        }

        if(l_pt == NULL) {
            const uint64_t l_pdEntry = l_pd[l_pdIndex];

            if((l_pd[l_pdIndex] & C_VMM_PAGING_FLAG_PRESENT) != 0) {
                l_pt = pmm_physicalToLinear(
                    (void *)(l_pdEntry & 0x000ffffffffff000UL)
                );
            } else {
                uint64_t *l_ptPhysical = pmm_alloc(C_MM_PAGE_SIZE);

                if(l_ptPhysical == NULL) {
                    error = true;
                    break;
                }

                l_pt = pmm_physicalToLinear(l_ptPhysical);
                memset(l_pt, 0, C_MM_PAGE_SIZE);

                l_pd[l_pdIndex] = (uint64_t)l_ptPhysical | l_pdFlags;
            }
        }

        l_pt[l_ptIndex] =
            ((uint64_t)l_physicalRange.m_ptr + l_currentOffset) | l_ptFlags;

        l_currentOffset += C_MM_PAGE_SIZE;
        l_ptIndex++;
        l_remainingPages--;

        if(l_ptIndex == 512UL) {
            l_pdIndex++;
            l_ptIndex = 0UL;
            l_pt = NULL;

            if(l_pdIndex == 512UL) {
                l_pdptIndex++;
                l_pdIndex = 0UL;
                l_pd = NULL;

                if(l_pdptIndex == 512UL) {
                    l_pml4Index++;
                    l_pdptIndex = 0UL;
                    l_pdpt = NULL;
                }
            }
        }
    }

    if(error) {
        // Unmap pages
        vmm_unmap(p_context, p_vptr, l_currentOffset);
    }

    spinlock_release(&p_context->m_spinlock);

    if(error) {
        return -ENOMEM;
    } else {
        pr_info(
            "vmm: Mapped [0x%016lx -> 0x%016lx] (0x%lx bytes)\n",
            l_physicalRange.m_ptr,
            l_virtualRange.m_ptr,
            l_physicalRange.m_size
        );
        return 0;
    }
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
    g_vmm_kernelContext.m_pagingContext =
        vmm_getPhysicalAddress(&s_vmm_kernelPml4);
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
    const struct ts_bootstrap_information *l_bootstrapInformation =
        bootstrap_getInformation();
    const struct ts_bootstrap_memoryMap *l_memoryMap =
        &l_bootstrapInformation->m_memoryMap;

    for(size_t l_i = 0; l_i < l_memoryMap->m_memoryMapLength; l_i++) {
        const struct ts_memoryRange *l_range =
            &l_memoryMap->m_memoryMap[l_i].m_range;

        if(
            vmm_map(
                &g_vmm_kernelContext,
                (void *)((uintptr_t)l_range->m_ptr + 0xffff800000000000),
                l_range->m_ptr,
                l_range->m_size,
                C_VMM_PROT_READ_WRITE | C_VMM_CACHE_WRITEBACK
            )
        ) {
            panic("Failed to initialize HHDM.\n");
        }
    }
}

static void vmm_mapKernel(void) {
    const struct ts_bootstrap_information *l_bootstrapInformation =
        bootstrap_getInformation();

    pr_info("vmm: Kernel located at 0x%016lx in RAM.\n", l_bootstrapInformation->m_kernelAddress.m_physicalAddress);
    pr_info("vmm: Kernel located at 0x%016lx.\n", l_bootstrapInformation->m_kernelAddress.m_virtualAddress);

    const size_t l_kernelTextOffset = (size_t)(
        ((uintptr_t)&g_kernelTextStart) - ((uintptr_t)&g_kernelStart)
    );
    const size_t l_kernelDataOffset = (size_t)(
        ((uintptr_t)&g_kernelDataStart) - ((uintptr_t)&g_kernelStart)
    );
    const size_t l_kernelRodataOffset = (size_t)(
        ((uintptr_t)&g_kernelRodataStart) - ((uintptr_t)&g_kernelStart)
    );
    const size_t l_kernelBssOffset = (size_t)(
        ((uintptr_t)&g_kernelBssStart) - ((uintptr_t)&g_kernelStart)
    );
    void *const l_physicalText = (void *)(
        (uintptr_t)l_bootstrapInformation->m_kernelAddress.m_physicalAddress
        + l_kernelTextOffset
    );
    void *const l_physicalData = (void *)(
        (uintptr_t)l_bootstrapInformation->m_kernelAddress.m_physicalAddress
        + l_kernelDataOffset
    );
    void *const l_physicalRodata = (void *)(
        (uintptr_t)l_bootstrapInformation->m_kernelAddress.m_physicalAddress
        + l_kernelRodataOffset
    );
    void *const l_physicalBss = (void *)(
        (uintptr_t)l_bootstrapInformation->m_kernelAddress.m_physicalAddress
        + l_kernelBssOffset
    );
    const size_t l_textSize = (size_t)(
        ((uintptr_t)&g_kernelTextEnd) - ((uintptr_t)&g_kernelTextStart)
    );
    const size_t l_dataSize = (size_t)(
        ((uintptr_t)&g_kernelDataEnd) - ((uintptr_t)&g_kernelDataStart)
    );
    const size_t l_rodataSize = (size_t)(
        ((uintptr_t)&g_kernelRodataEnd) - ((uintptr_t)&g_kernelRodataStart)
    );
    const size_t l_bssSize = (size_t)(
        ((uintptr_t)&g_kernelBssEnd) - ((uintptr_t)&g_kernelBssStart)
    );

    // Map .text as RX
    if(
        vmm_map(
            &g_vmm_kernelContext,
            &g_kernelTextStart,
            l_physicalText,
            l_textSize,
            C_VMM_PROT_RX | C_VMM_CACHE_WRITEPROTECT
        ) != 0
    ) {
        panic("Failed to map kernel .text section.\n");
    }

    // Map .data as RW
    if(
        vmm_map(
            &g_vmm_kernelContext,
            &g_kernelDataStart,
            l_physicalData,
            l_dataSize,
            C_VMM_PROT_RW | C_VMM_CACHE_WRITEBACK
        ) != 0
    ) {
        panic("Failed to map kernel .data section.\n");
    }

    // Map .rodata as RO
    if(
        vmm_map(
            &g_vmm_kernelContext,
            &g_kernelRodataStart,
            l_physicalRodata,
            l_rodataSize,
            C_VMM_PROT_RO | C_VMM_CACHE_WRITEPROTECT
        ) != 0
    ) {
        panic("Failed to map kernel .rodata section.\n");
    }

    // Map .bss as RW
    if(
        vmm_map(
            &g_vmm_kernelContext,
            &g_kernelBssStart,
            l_physicalBss,
            l_bssSize,
            C_VMM_PROT_RW | C_VMM_CACHE_WRITEBACK
        ) != 0
    ) {
        panic("Failed to map kernel .bss section.\n");
    }
}

static void vmm_switchToKernelContext(void) {
    pr_info("vmm: Switching to kernel context\n");
    pr_info("vmm: Setting CR3 to 0x%016lx\n", g_vmm_kernelContext.m_pagingContext);
    asm_writeCr3((uint64_t)g_vmm_kernelContext.m_pagingContext);
    g_pmm_hhdm = (void *)0xffff800000000000;
}
