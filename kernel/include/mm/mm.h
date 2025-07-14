#ifndef __INCLUDE_MM_MM_H__
#define __INCLUDE_MM_MM_H__

#include <stddef.h>

#define C_MM_PAGE_SIZE 4096

struct ts_mm_memoryMapEntry {
    void *m_base;
    size_t m_size;
};

struct ts_mm_memoryMapEntryListNode {
    struct ts_mm_memoryMapEntryListNode *m_next;
    struct ts_mm_memoryMapEntry m_data;
};

size_t mm_roundDownPage(size_t p_size);
size_t mm_roundUpPage(size_t p_size);
void *mm_getEntryEndAddress(const struct ts_mm_memoryMapEntry *p_entry);
void mm_tryMergeNodes(
    struct ts_mm_memoryMapEntryListNode *p_node,
    void (*p_freeNode)(
        void *p_context,
        struct ts_mm_memoryMapEntryListNode *p_node
    ),
    void *p_freeNodeContext
);
void *mm_alloc(struct ts_mm_memoryMapEntryListNode **p_map, size_t p_size);
void mm_addNodeToMap(
    struct ts_mm_memoryMapEntryListNode **p_map,
    struct ts_mm_memoryMapEntryListNode *p_node,
    void (*p_freeNode)(
        void *p_context,
        struct ts_mm_memoryMapEntryListNode *p_node
    ),
    void *p_freeNodeContext
);

#endif
