#include <stddef.h>
#include <stdint.h>

#include "kernel/mm/mm.h"

size_t mmRoundDownPage(size_t p_size) {
    return p_size & ~(C_MM_PAGE_SIZE - 1);
}

size_t mmRoundUpPage(size_t p_size) {
    size_t l_roundedDownSize = mmRoundDownPage(p_size);

    if(l_roundedDownSize != p_size) {
        return l_roundedDownSize + C_MM_PAGE_SIZE;
    }

    return p_size;
}

void *mmGetEntryEndAddress(const struct ts_mmMemoryMapEntry *p_entry) {
    return (void *)((uintptr_t)p_entry->m_base + p_entry->m_size);
}

void mmTryMergeNodes(
    struct ts_mmMemoryMapEntryListNode *p_node,
    void (*p_freeNode)(
        void *p_context,
        struct ts_mmMemoryMapEntryListNode *p_node
    ),
    void *p_freeNodeContext
) {
    // If there is no next node, then there is nothing to merge.
    if(p_node->m_next == NULL) {
        return;
    }

    struct ts_mmMemoryMapEntryListNode *l_nextNode = p_node->m_next;

    void *l_entryEndAddress = mmGetEntryEndAddress(&p_node->m_data);

    if(l_entryEndAddress == p_node->m_next) {
        struct ts_mmMemoryMapEntryListNode *l_nodeToFree = p_node->m_next;

        p_node->m_data.m_size += l_nextNode->m_data.m_size;
        p_node->m_next = p_node->m_next->m_next;

        if(p_freeNode != NULL) {
            p_freeNode(l_nodeToFree, p_freeNodeContext);
        }
    }
}

void *mmAlloc(struct ts_mmMemoryMapEntryListNode **p_map, size_t p_size) {
    size_t l_size = mmRoundUpPage(p_size);

    struct ts_mmMemoryMapEntryListNode *l_previousNode = NULL;
    struct ts_mmMemoryMapEntryListNode *l_node = *p_map;

    while(l_node != NULL) {
        if(l_node->m_data.m_size < l_size) {
            l_previousNode = l_node;
            l_node = l_node->m_next;
            continue;
        } else if(l_node->m_data.m_size == l_size) {
            if(l_node == *p_map) {
                *p_map = (*p_map)->m_next;
            } else {
                l_previousNode->m_next = l_node->m_next;
            }

            return l_node;
        } else {
            uintptr_t l_startAddress =
                (uintptr_t)l_node->m_data.m_base + l_node->m_data.m_size
                - l_size;

            l_node->m_data.m_size -= l_size;

            return (void *)l_startAddress;
        }
    }

    return NULL;
}

void mmAddNodeToMap(
    struct ts_mmMemoryMapEntryListNode **p_map,
    struct ts_mmMemoryMapEntryListNode *p_node,
    void (*p_freeNode)(
        void *p_context,
        struct ts_mmMemoryMapEntryListNode *p_node
    ),
    void *p_freeNodeContext
) {
    struct ts_mmMemoryMapEntryListNode *l_node = *p_map;
    struct ts_mmMemoryMapEntryListNode *l_previousNode = NULL;

    while(l_node != NULL && l_node->m_data.m_base < p_node->m_data.m_base) {
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
    mmTryMergeNodes(p_node, p_freeNode, p_freeNodeContext);

    if(l_previousNode != NULL) {
        // Try to merge the previous node with the new node
        mmTryMergeNodes(l_previousNode, p_freeNode, p_freeNodeContext);
    }
}
