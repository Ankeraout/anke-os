#ifndef __INCLUDE_MM_MM_H__
#define __INCLUDE_MM_MM_H__

#include <stddef.h>

#include "memoryRange.h"

#define C_MM_PAGE_SIZE 4096

size_t mm_roundDownPage(size_t p_size);
size_t mm_roundUpPage(size_t p_size);
void *mm_getEntryEndAddress(const struct ts_memoryRange *p_entry);
void mm_tryMergeNodes(
    struct ts_memoryRange_listNode *p_node,
    void (*p_freeNode)(
        void *p_context,
        struct ts_memoryRange_listNode *p_node
    ),
    void *p_freeNodeContext
);
void *mm_alloc(struct ts_memoryRange_listNode **p_map, size_t p_size);
void *mm_allocPages(struct ts_memoryRange_listNode **p_map, size_t p_size);
void mm_addNodeToMap(
    struct ts_memoryRange_listNode **p_map,
    struct ts_memoryRange_listNode *p_node,
    void (*p_freeNode)(
        void *p_context,
        struct ts_memoryRange_listNode *p_node
    ),
    void *p_freeNodeContext
);
void mm_alignRange(struct ts_memoryRange *p_memoryRange);

#endif
