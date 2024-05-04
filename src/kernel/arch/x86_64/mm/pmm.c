#include "kernel/mm/mm.h"
#include "kernel/mm/pmm.h"

static struct ts_mmMemoryMapEntryListNode *s_freeMemoryEntryList;

int pmmInit(
    const struct ts_mmMemoryMapEntry *p_memoryMap,
    int p_memoryMapEntryCount
) {
    s_freeMemoryEntryList = NULL;

    for(int l_index = p_memoryMapEntryCount - 1; l_index >= 0; l_index--) {
        if(p_memoryMap[l_index].m_size < C_MM_PAGE_SIZE) {
            // We cannot use free memory sections that are smaller than a page.
            continue;
        }

        // Initialize the corresponding entry in the list
        struct ts_mmMemoryMapEntryListNode *l_node =
            (struct ts_mmMemoryMapEntryListNode *)p_memoryMap[l_index].m_base;

        l_node->m_data.m_base = p_memoryMap[l_index].m_base;
        l_node->m_data.m_size = p_memoryMap[l_index].m_size;
        l_node->m_next = s_freeMemoryEntryList;
        s_freeMemoryEntryList = l_node;
    }

    return 0;
}

void *pmmAlloc(size_t p_size) {
    return mmAlloc(&s_freeMemoryEntryList, p_size);
}

void pmmFree(void *p_ptr, size_t p_size) {
    size_t l_size = mmRoundUpPage(p_size);
    struct ts_mmMemoryMapEntryListNode *l_newNode =
        (struct ts_mmMemoryMapEntryListNode *)p_ptr;

    l_newNode->m_data.m_base = p_ptr;
    l_newNode->m_data.m_size = l_size;

    mmAddNodeToMap(&s_freeMemoryEntryList, l_newNode, NULL, NULL);
}
