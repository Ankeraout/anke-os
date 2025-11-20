#include <stdbool.h>

#include "bootstrap.h"
#include "mm/mm.h"
#include "mm/pmm.h"
#include "spinlock.h"

static void pmm_tryMergeNodes(
    struct ts_memoryRange_listNode *p_node,
    struct ts_memoryRange_listNode *p_nextNode
);

static struct ts_memoryRange_listNode *s_freeMemoryEntryList;
static t_spinlock s_spinlock;

int pmm_init(void) {
    const struct ts_bootstrap_information *l_bootstrapInformation =
        bootstrap_getInformation();
    const struct ts_bootstrap_memoryMapEntry *l_memoryMap =
        l_bootstrapInformation->m_memoryMap.m_memoryMap;
    const size_t l_memoryMapLength =
        l_bootstrapInformation->m_memoryMap.m_memoryMapLength;

    s_freeMemoryEntryList = NULL;

    for(int l_index = l_memoryMapLength - 1; l_index >= 0; l_index--) {
        // Round start to the nearest page
        size_t l_startAddress =
            mm_roundUpPage((size_t)l_memoryMap[l_index].m_range.m_ptr);
        size_t l_rangeStartOffset =
            (size_t)l_memoryMap[l_index].m_range.m_ptr - l_startAddress;

        // The entry does not contain a full page.
        if(l_memoryMap[l_index].m_range.m_size < l_rangeStartOffset) {
            continue;
        }

        size_t l_size =
            mm_roundDownPage(
                l_memoryMap[l_index].m_range.m_size
                - (l_startAddress - (size_t)l_memoryMap[l_index].m_range.m_ptr)
            );

        if(l_size == 0) {
            // The entry does not contain a full page.
            continue;
        }

        // Initialize the corresponding entry in the list
        struct ts_memoryRange_listNode *l_node = pmm_physicalToLinear(
            (struct ts_memoryRange_listNode *)l_startAddress
        );

        l_node->m_memoryRange.m_ptr = (void *)l_startAddress;
        l_node->m_memoryRange.m_size = l_size;
        l_node->m_next = s_freeMemoryEntryList;
        s_freeMemoryEntryList = l_node;
    }

    return 0;
}

void *pmm_alloc(size_t p_size) {
    void *l_returnValue = NULL;

    spinlock_acquire(&s_spinlock);

    size_t l_size = mm_roundUpPage(p_size);

    // Look for entries with size >= l_size.

    bool l_found = false;
    struct ts_memoryRange_listNode *l_previousNode = NULL;
    struct ts_memoryRange_listNode *l_node = s_freeMemoryEntryList;

    while((!l_found) && (l_node != NULL)) {
        if(l_node->m_memoryRange.m_size >= l_size) {
            l_found = true;
        } else {
            l_previousNode = l_node;
            l_node = l_node->m_next;
        }
    }

    if(l_found) {
        size_t l_startAddress = (size_t)l_node->m_memoryRange.m_ptr
            + l_node->m_memoryRange.m_size - l_size;
        
        if(l_node->m_memoryRange.m_size == l_size) {
            if(l_previousNode == NULL) {
                s_freeMemoryEntryList = l_node->m_next;
            } else {
                l_previousNode->m_next = l_node->m_next;
            }
        } else {
            l_node->m_memoryRange.m_size -= l_size;
        }

        l_returnValue = (void *)l_startAddress;
    }

    spinlock_release(&s_spinlock);

    return l_returnValue;
}

void pmm_free(void *p_ptr, size_t p_size) {
    size_t l_size = mm_roundUpPage(p_size);
    
    struct ts_memoryRange_listNode *l_newNode = pmm_physicalToLinear(
        (struct ts_memoryRange_listNode *)p_ptr
    );

    l_newNode->m_memoryRange.m_ptr = p_ptr;
    l_newNode->m_memoryRange.m_size = l_size;

    spinlock_acquire(&s_spinlock);

    // Locate the previous and next nodes.
    struct ts_memoryRange_listNode *l_previousNode = NULL;
    struct ts_memoryRange_listNode *l_nextNode = s_freeMemoryEntryList;

    while((l_nextNode != NULL) && (l_nextNode->m_memoryRange.m_ptr < p_ptr)) {
        l_previousNode = l_nextNode;
        l_nextNode = l_nextNode->m_next;
    }

    // Add the node to the ordered node list.
    l_newNode->m_next = l_nextNode;

    if(l_previousNode == NULL) {
        s_freeMemoryEntryList = l_newNode;
    } else {
        l_previousNode->m_next = l_newNode;
    }

    // Try to merge with the previous node.
    if(l_previousNode != NULL) {
        pmm_tryMergeNodes(l_previousNode, l_newNode);
    }

    // Try to merge with the next node.
    if(l_nextNode != NULL) {
        pmm_tryMergeNodes(l_newNode, l_nextNode);
    }

    spinlock_release(&s_spinlock);
}

static void pmm_tryMergeNodes(
    struct ts_memoryRange_listNode *p_node,
    struct ts_memoryRange_listNode *p_nextNode
) {
    void *l_endPtr = (void *)(
        (uintptr_t)p_node->m_memoryRange.m_ptr
        + p_node->m_memoryRange.m_size
    );

    if(l_endPtr == p_nextNode->m_memoryRange.m_ptr) {
        p_node->m_next = p_nextNode->m_next;
        p_node->m_memoryRange.m_size += p_nextNode->m_memoryRange.m_size;
    }
}
