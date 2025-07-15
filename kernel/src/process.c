#include <stdlib.h>

#include "criticalSection.h"
#include "list.h"
#include "memoryAllocation.h"
#include "mm/pmm.h"
#include "process.h"
#include "string.h"
#include "thread.h"

static pid_t s_nextPid;

struct ts_process *process_new(struct ts_process *p_parent) {
    criticalSection_enter();

    struct ts_process *l_process = malloc(sizeof(struct ts_process));
    
    if(l_process == NULL) {
        goto errorFailedToCreateProcess;
    }

    memset(l_process, 0, sizeof(struct ts_process));

    if(list_insertBeginning(&l_process->m_childList, l_process) != 0) {
        goto errorFailedToAddChildProcess;
    }

    l_process->m_memoryContext = vmm_createContext();

    if(l_process->m_memoryContext == NULL) {
        goto errorFailedToCreateMemoryContext;
    }

    l_process->m_id = s_nextPid++;
    l_process->m_parent = p_parent;

    criticalSection_leave();
    return l_process;

    errorFailedToCreateMemoryContext:
    list_remove(&l_process->m_childList, l_process);

    errorFailedToAddChildProcess:
    free(l_process);

    errorFailedToCreateProcess:
    criticalSection_leave();
    return NULL;
}

void process_destroy(struct ts_process *p_process) {
    // Destroy threads
    struct ts_list_node *l_node = p_process->m_threadList;

    while(l_node != NULL) {
        struct ts_thread *l_thread =
            (struct ts_thread *)l_node->m_data;

        thread_destroy(l_thread);

        struct ts_list_node *l_nextNode = l_node->m_next;

        free(l_node);

        l_node = l_nextNode;
    }

    // Destroy virtual memory allocations
    l_node = p_process->m_virtualMemoryAllocationList;

    while(l_node != NULL) {
        struct ts_memoryAllocation *l_memoryAllocation =
            (struct ts_memoryAllocation *)l_node->m_data;

        vmm_free(
            p_process->m_memoryContext,
            l_memoryAllocation->m_ptr,
            l_memoryAllocation->m_size
        );

        struct ts_list_node *l_nextNode = l_node->m_next;

        free(l_node);

        l_node = l_nextNode;
    }

    // Destroy physical memory allocations
    l_node = p_process->m_physicalMemoryAllocationList;

    while(l_node != NULL) {
        struct ts_memoryAllocation *l_memoryAllocation =
            (struct ts_memoryAllocation *)l_node->m_data;

        pmm_free(l_memoryAllocation->m_ptr, l_memoryAllocation->m_size);

        struct ts_list_node *l_nextNode = l_node->m_next;

        free(l_node);

        l_node = l_nextNode;
    }

    // Destroy memory context
    vmm_destroyContext(p_process->m_memoryContext);

    // Destroy process
    free(p_process);
}
