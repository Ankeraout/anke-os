#include "mm/pmm.h"
#include "process.h"
#include "stdlib.h"
#include "thread.h"

struct ts_process *process_new(
    struct ts_process *p_parent,
    void *p_tdata,
    size_t p_tdataSize,
    size_t p_tbssSize
) {
    struct ts_process *l_process = malloc(sizeof(struct ts_process));

    if(l_process == NULL) {
        return NULL;
    }

    l_process->m_memoryContext = vmm_createContext();

    if(l_process->m_memoryContext == NULL) {
        free(l_process);
        return NULL;
    }

    l_process->m_pid = 0; // TODO
    l_process->m_parent = p_parent;
    l_process->m_memoryRangeList = NULL;
    l_process->m_threadList = NULL;
    l_process->m_childList = NULL;
    l_process->m_tdata = p_tdata;
    l_process->m_tdataSize = p_tdataSize;
    l_process->m_tbssSize = p_tbssSize;

    return l_process;
}

void process_destroy(struct ts_process *p_process) {
    // Destroy threads
    struct ts_list_node *l_node = p_process->m_threadList;

    while(l_node != NULL) {
        struct ts_thread *l_thread = (struct ts_thread *)l_node->m_data;
        thread_destroy(l_thread);
        struct ts_list_node *l_nextNode = l_node->m_next;
        free(l_node);
        l_node = l_nextNode;
    }

    // Make parent adopt children
    struct ts_list_node *l_previousNode = NULL;
    l_node = p_process->m_childList;

    while(l_node != NULL) {
        struct ts_process *l_process = (struct ts_process *)l_node->m_data;
        l_process->m_parent = p_process->m_parent;
        l_previousNode = l_node;
        l_node = l_node->m_next;
    }

    if(l_previousNode != NULL) {
        l_previousNode->m_next = p_process->m_parent->m_childList;
        p_process->m_parent->m_childList = p_process->m_childList;
    }

    // Free memory allocations
    struct ts_memoryRange_listNode *l_memoryRangeNode =
        p_process->m_memoryRangeList;

    while(l_memoryRangeNode != NULL) {
        pmm_free(
            l_memoryRangeNode->m_memoryRange.m_ptr,
            l_memoryRangeNode->m_memoryRange.m_size
        );

        struct ts_memoryRange_listNode *l_nextNode = l_memoryRangeNode;
        free(l_memoryRangeNode);
        l_memoryRangeNode = l_nextNode;
    }

    // Free tdata
    free(p_process->m_tdata);

    // Destroy context
    vmm_destroyContext(p_process->m_memoryContext);

    // Free process
    free(p_process);
}
