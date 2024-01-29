#ifndef __INCLUDE_KERNEL_MM_MM_H__
#define __INCLUDE_KERNEL_MM_MM_H__

#include <stddef.h>

#define C_MM_PAGE_SIZE 4096

struct ts_mmMemoryMapEntry {
    void *m_base;
    size_t m_size;
};

struct ts_mmMemoryMapEntryListNode {
    struct ts_mmMemoryMapEntryListNode *m_next;
    struct ts_mmMemoryMapEntry m_data;
};

size_t mmRoundDownPage(size_t p_size);
size_t mmRoundUpPage(size_t p_size);
void *mmGetEntryEndAddress(const struct ts_mmMemoryMapEntry *p_entry);
void mmTryMergeNodes(
    struct ts_mmMemoryMapEntryListNode *p_node,
    void (*p_freeNode)(
        void *p_context,
        struct ts_mmMemoryMapEntryListNode *p_node
    ),
    void *p_freeNodeContext
);
void *mmAlloc(struct ts_mmMemoryMapEntryListNode **p_map, size_t p_size);
void mmAddNodeToMap(
    struct ts_mmMemoryMapEntryListNode **p_map,
    struct ts_mmMemoryMapEntryListNode *p_node,
    void (*p_freeNode)(
        void *p_context,
        struct ts_mmMemoryMapEntryListNode *p_node
    ),
    void *p_freeNodeContext
);

#endif
