#include "mm/mm.h"
#include "mm/pmm.h"
#include "spinlock.h"

static struct ts_memoryRange_listNode *s_freeMemoryEntryList;
static t_spinlock s_spinlock;

int pmm_init(
    const struct ts_memoryRange *p_memoryMap,
    int p_memoryMapEntryCount
) {
    s_freeMemoryEntryList = NULL;

    for(int l_index = p_memoryMapEntryCount - 1; l_index >= 0; l_index--) {
        if(p_memoryMap[l_index].m_size < C_MM_PAGE_SIZE) {
            // We cannot use free memory sections that are smaller than a page.
            continue;
        }

        // Initialize the corresponding entry in the list
        struct ts_memoryRange_listNode *l_node =
            (struct ts_memoryRange_listNode *)p_memoryMap[l_index].m_ptr;

        l_node->m_memoryRange.m_ptr = p_memoryMap[l_index].m_ptr;
        l_node->m_memoryRange.m_size = p_memoryMap[l_index].m_size;
        l_node->m_next = s_freeMemoryEntryList;
        s_freeMemoryEntryList = l_node;
    }

    return 0;
}

void *pmm_alloc(size_t p_size) {
    spinlock_acquire(&s_spinlock);

    void *l_returnValue = mm_alloc(&s_freeMemoryEntryList, p_size);

    spinlock_release(&s_spinlock);

    return l_returnValue;
}

void pmm_free(void *p_ptr, size_t p_size) {
    size_t l_size = mm_roundUpPage(p_size);
    struct ts_memoryRange_listNode *l_newNode =
        (struct ts_memoryRange_listNode *)p_ptr;

    l_newNode->m_memoryRange.m_ptr = p_ptr;
    l_newNode->m_memoryRange.m_size = l_size;

    spinlock_acquire(&s_spinlock);

    mm_addNodeToMap(&s_freeMemoryEntryList, l_newNode, NULL, NULL);

    spinlock_release(&s_spinlock);
}
