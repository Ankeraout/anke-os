#ifndef __INCLUDE_MEMORYRANGE_H__
#define __INCLUDE_MEMORYRANGE_H__

#include <stddef.h>

#include "list.h"

struct ts_memoryRange {
    void *m_ptr;
    size_t m_size;
};

struct ts_memoryRange_listNode {
    struct ts_memoryRange_listNode *m_next;
    struct ts_memoryRange m_memoryRange;
};

#endif
