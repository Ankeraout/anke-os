#include <stddef.h>
#include <stdint.h>

#include "mm/mm.h"

size_t mm_roundDownPage(size_t p_size) {
    return p_size & ~(C_MM_PAGE_SIZE - 1);
}

size_t mm_roundUpPage(size_t p_size) {
    size_t l_roundedDownSize = mm_roundDownPage(p_size);

    if(l_roundedDownSize != p_size) {
        return l_roundedDownSize + C_MM_PAGE_SIZE;
    }

    return p_size;
}

void *mm_getEntryEndAddress(const struct ts_memoryRange *p_entry) {
    return (void *)((uintptr_t)p_entry->m_ptr + p_entry->m_size);
}

void mm_tryMergeNodes(
    struct ts_memoryRange_listNode *p_node,
    void (*p_freeNode)(
        void *p_context,
        struct ts_memoryRange_listNode *p_node
    ),
    void *p_freeNodeContext
) {
    // If there is no next node, then there is nothing to merge.
    if(p_node->m_next == NULL) {
        return;
    }

    struct ts_memoryRange_listNode *l_nextNode = p_node->m_next;

    void *l_entryEndAddress = mm_getEntryEndAddress(&p_node->m_memoryRange);

    if(l_entryEndAddress == p_node->m_next) {
        struct ts_memoryRange_listNode *l_nodeToFree = p_node->m_next;

        p_node->m_memoryRange.m_size += l_nextNode->m_memoryRange.m_size;
        p_node->m_next = p_node->m_next->m_next;

        if(p_freeNode != NULL) {
            p_freeNode(l_nodeToFree, p_freeNodeContext);
        }
    }
}

void *mm_alloc(struct ts_memoryRange_listNode **p_map, size_t p_size) {
    struct ts_memoryRange_listNode *l_previousNode = NULL;
    struct ts_memoryRange_listNode *l_node = *p_map;

    while(l_node != NULL) {
        if(l_node->m_memoryRange.m_size < p_size) {
            l_previousNode = l_node;
            l_node = l_node->m_next;
            continue;
        } else if(l_node->m_memoryRange.m_size == p_size) {
            if(l_node == *p_map) {
                *p_map = (*p_map)->m_next;
            } else {
                l_previousNode->m_next = l_node->m_next;
            }

            return l_node;
        } else {
            uintptr_t l_startAddress =
                (uintptr_t)l_node->m_memoryRange.m_ptr
                + l_node->m_memoryRange.m_size
                - p_size;

            l_node->m_memoryRange.m_size -= p_size;

            return (void *)l_startAddress;
        }
    }

    return NULL;
}

void *mm_allocPages(struct ts_memoryRange_listNode **p_map, size_t p_size) {
    return mm_alloc(p_map, mm_roundUpPage(p_size));
}

void mm_addNodeToMap(
    struct ts_memoryRange_listNode **p_map,
    struct ts_memoryRange_listNode *p_node,
    void (*p_freeNode)(
        void *p_context,
        struct ts_memoryRange_listNode *p_node
    ),
    void *p_freeNodeContext
) {
    struct ts_memoryRange_listNode *l_node = *p_map;
    struct ts_memoryRange_listNode *l_previousNode = NULL;

    while(
        (l_node != NULL)
        && (l_node->m_memoryRange.m_ptr < p_node->m_memoryRange.m_ptr)
    ) {
        l_previousNode = l_node;
        l_node = l_node->m_next;
    }

    if(l_previousNode == NULL) {
        p_node->m_next = *p_map;
        *p_map = p_node;
    } else {
        l_previousNode->m_next = p_node;
        p_node->m_next = l_node;
    }

    // Try to merge the next node with the new node
    mm_tryMergeNodes(p_node, p_freeNode, p_freeNodeContext);

    if(l_previousNode != NULL) {
        // Try to merge the previous node with the new node
        mm_tryMergeNodes(l_previousNode, p_freeNode, p_freeNodeContext);
    }
}

void mm_alignRange(struct ts_memoryRange *p_memoryRange) {
    size_t l_pageOffset = ((uintptr_t)p_memoryRange->m_ptr) & 0xfffUL;
    p_memoryRange->m_ptr =
        (void *)(((uintptr_t)p_memoryRange->m_ptr) - l_pageOffset);
    p_memoryRange->m_size =
        mm_roundUpPage(p_memoryRange->m_size + l_pageOffset);
}
