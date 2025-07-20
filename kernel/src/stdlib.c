#include "errno.h"
#include "memoryRange.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "stdlib.h"
#include "printk.h"

union tu_malloc_blockHeader {
    size_t m_size;
    struct ts_memoryRange_listNode m_node;
};

static struct ts_memoryRange_listNode *s_mallocFreePhysicalMemoryPool;

static int malloc_refillPhysicalMemoryPool(size_t p_size);

void *malloc(size_t p_size) {
    size_t l_sizeToAllocate = p_size + sizeof(size_t);

    if(l_sizeToAllocate < sizeof(union tu_malloc_blockHeader)) {
        l_sizeToAllocate = sizeof(union tu_malloc_blockHeader);
    }

    if((l_sizeToAllocate & 7UL) != 0UL) {
        l_sizeToAllocate += 8UL;
        l_sizeToAllocate &= ~7UL;
    }

    void *l_result =
        mm_alloc(&s_mallocFreePhysicalMemoryPool, l_sizeToAllocate);

    if(l_result == NULL) {
        if(malloc_refillPhysicalMemoryPool(l_sizeToAllocate) != 0) {
            return NULL;
        }

        l_result = mm_alloc(&s_mallocFreePhysicalMemoryPool, l_sizeToAllocate);
    }

    if(l_result == NULL) {
        return NULL;
    }

    union tu_malloc_blockHeader *l_header =
        (union tu_malloc_blockHeader *)l_result;

    l_header->m_size = l_sizeToAllocate;

    return (void *)((uintptr_t)&l_header->m_size + sizeof(size_t));
}

void free(void *p_ptr) {
    union tu_malloc_blockHeader *l_blockHeader =
        (union tu_malloc_blockHeader *)((uintptr_t)p_ptr - sizeof(size_t));

    l_blockHeader->m_node.m_memoryRange.m_ptr = l_blockHeader;
    l_blockHeader->m_node.m_memoryRange.m_size = l_blockHeader->m_size;

    mm_addNodeToMap(
        &s_mallocFreePhysicalMemoryPool,
        &l_blockHeader->m_node,
        NULL,
        NULL
    );
}

static int malloc_refillPhysicalMemoryPool(size_t p_size) {
    struct ts_vmm_context *l_kernelContext = vmm_getKernelContext();

    void *l_virtualPtr = vmm_alloc(
        l_kernelContext,
        p_size,
        C_VMM_ALLOC_FLAG_KERNEL
    );

    if(l_virtualPtr == NULL) {
        return -ENOMEM;
    }

    size_t l_allocBlockSize = mm_roundUpPage(p_size);
    size_t l_allocatedPhysicalMemory = 0;

    while(l_allocatedPhysicalMemory < p_size) {
        void *l_physicalPtr = pmm_alloc(l_allocBlockSize);

        if(l_physicalPtr == NULL) {
            if(l_allocBlockSize == C_MM_PAGE_SIZE) {
                break;
            }

            l_allocBlockSize >>= 1UL;

            continue;
        }

        if(
            vmm_map(
                l_kernelContext,
                (void *)((uintptr_t)l_virtualPtr + l_allocatedPhysicalMemory),
                l_physicalPtr,
                l_allocBlockSize,
                C_VMM_PROT_KERNEL | C_VMM_PROT_EXEC | C_VMM_PROT_READ_WRITE
            ) != 0
        ) {
            pmm_free(l_physicalPtr, l_allocBlockSize);
            break;
        }

        l_allocatedPhysicalMemory += l_allocBlockSize;
    }

    if(l_allocatedPhysicalMemory < p_size) {
        while(l_allocatedPhysicalMemory != 0UL) {
            pmm_free(
                vmm_getPhysicalAddress(
                    (void *)(
                        (uintptr_t)l_virtualPtr + l_allocatedPhysicalMemory
                    )
                ),
                C_MM_PAGE_SIZE
            );

            l_allocatedPhysicalMemory -= C_MM_PAGE_SIZE;
        }

        vmm_free(l_kernelContext, l_virtualPtr, p_size);

        return -ENOMEM;
    }

    union tu_malloc_blockHeader *l_header =
        (union tu_malloc_blockHeader *)l_virtualPtr;

    l_header->m_node.m_memoryRange.m_ptr = l_virtualPtr;
    l_header->m_node.m_memoryRange.m_size = l_allocatedPhysicalMemory;
    l_header->m_node.m_next = s_mallocFreePhysicalMemoryPool;

    s_mallocFreePhysicalMemoryPool = &l_header->m_node;

    return 0;
}
