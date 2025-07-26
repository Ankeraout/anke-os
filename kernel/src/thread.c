#include "mm/pmm.h"
#include "stdlib.h"
#include "string.h"
#include "thread.h"

struct ts_thread *thread_new(struct ts_process *p_process, size_t p_stackSize) {
    struct ts_thread *l_thread = malloc(sizeof(struct ts_thread));

    if(l_thread == NULL) {
        return NULL;
    }

    l_thread->m_memoryRangeList = NULL;
    l_thread->m_tbss = vmm_allocAndMap(
        p_process->m_memoryContext,
        &l_thread->m_memoryRangeList,
        p_process->m_tbssSize,
        0,
        C_VMM_PROT_USER | C_VMM_PROT_READ_WRITE
    );

    if(l_thread->m_tbss == NULL) {
        free(l_thread);
        return NULL;
    }

    l_thread->m_tdata = vmm_allocAndMap(
        p_process->m_memoryContext,
        &l_thread->m_memoryRangeList,
        p_process->m_tdataSize,
        0,
        C_VMM_PROT_USER | C_VMM_PROT_READ_WRITE
    );

    if(l_thread->m_tdata == NULL) {
        // TODO : destroy tbss
        free(l_thread);
        return NULL;
    }

    memcpy(l_thread->m_tdata, p_process->m_tdata, p_process->m_tdataSize);

    l_thread->m_state = E_THREAD_STATE_INITIALIZING;
    l_thread->m_process = p_process;

    return l_thread;
}

void thread_destroy(struct ts_thread *p_thread) {
    // Remove thread from process' list of threads
    list_remove(&p_thread->m_process->m_threadList, p_thread);

    // Destroy thread memory allocations
    struct ts_memoryRange_listNode *l_node = p_thread->m_memoryRangeList;

    while(l_node != NULL) {
        pmm_free(l_node->m_memoryRange.m_ptr, l_node->m_memoryRange.m_size);
        struct ts_memoryRange_listNode *l_nextNode = l_node->m_next;
        free(l_node);
        l_node = l_nextNode;
    }

    free(p_thread);
}
