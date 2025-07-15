#include "memoryAllocation.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "stdlib.h"

void *memoryAllocation_allocate(
    struct ts_vmm_context *p_context,
    struct ts_list_node **p_list,
    size_t p_size
) {
    // Allocate physical memory
    size_t l_remainingSize = p_size;
    struct ts_list_node *l_list = NULL;

    while(l_remainingSize != 0UL) {
        void *l_physicalAddress = pmm_alloc(C_MM_PAGE_SIZE);

        if(l_physicalAddress == NULL) {
            goto failedToAllocatePhysicalMemory;
        }

        struct ts_memoryAllocation *l_memoryAllocation =
            malloc(sizeof(struct ts_memoryAllocation));

        if(l_memoryAllocation == NULL) {
            pmm_free(l_physicalAddress, C_MM_PAGE_SIZE);
            goto failedToAllocatePhysicalMemory;
        }

        struct ts_list_node *l_listNode = malloc(sizeof(struct ts_list_node));

        if(l_listNode == NULL) {
            free(l_memoryAllocation);
            pmm_free(l_physicalAddress, C_MM_PAGE_SIZE);
            goto failedToAllocatePhysicalMemory;
        }

        l_listNode->m_data = l_memoryAllocation;
        l_listNode->m_next = l_list;
        l_list = l_listNode;
        l_memoryAllocation->m_ptr = l_physicalAddress;
        l_memoryAllocation->m_size = C_MM_PAGE_SIZE;
    }

    // Allocate virtual memory
    void *l_virtualMemory = vmm_alloc(p_context, p_size, 0);
    
    if(l_virtualMemory == NULL) {
        goto failedToAllocateVirtualMemory;
    }

    // Map physical memory to virtual memory
    struct ts_list_node *l_node = l_list;

    size_t l_mappedSize = 0UL;

    while(l_node != NULL) {
        struct ts_memoryAllocation *l_memoryAllocation =
            (struct ts_memoryAllocation *)l_node->m_data;

        int l_result = vmm_map(
            p_context,
            (void *)((uintptr_t)(l_virtualMemory) + l_mappedSize),
            l_memoryAllocation->m_ptr,
            l_memoryAllocation->m_size,
            C_VMM_PROT_USER | C_VMM_PROT_EXEC | C_VMM_PROT_READ_WRITE
        );

        if(l_result != 0) {
            goto failedToMapMemory;
        }

        l_mappedSize += l_memoryAllocation->m_size;

        l_node = l_node->m_next;
    }

    l_list->m_next = *p_list;
    *p_list = l_list;

    return l_virtualMemory;

    failedToMapMemory:
    vmm_unmap(p_context, l_virtualMemory, l_mappedSize);
    vmm_free(p_context, l_virtualMemory, p_size);

    failedToAllocateVirtualMemory:
    failedToAllocatePhysicalMemory:
    while(l_list != NULL) {
        struct ts_list_node *l_nextNode = l_list->m_next;
        struct ts_memoryAllocation *l_memoryAllocation =
            (struct ts_memoryAllocation *)l_list->m_data;

        pmm_free(l_memoryAllocation->m_ptr, l_memoryAllocation->m_size);
        free(l_memoryAllocation);
        free(l_list);

        l_list = l_nextNode;
    }

    return NULL;
}
