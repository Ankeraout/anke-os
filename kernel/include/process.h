#ifndef __INCLUDE_PROCESS_H__
#define __INCLUDE_PROCESS_H__

#include <stdint.h>

#include "list.h"
#include "sys/types.h"
#include "mm/vmm.h"

struct ts_process {
    pid_t m_id;
    struct ts_process *m_parent;
    struct ts_list_node *m_threadList;
    struct ts_list_node *m_childList;
    struct ts_vmm_context *m_memoryContext;
    struct ts_list_node *m_physicalMemoryAllocationList;
    size_t m_threadLocalStorageSize;
};

struct ts_process *process_new(struct ts_process *p_parent);
void process_destroy(struct ts_process *p_process);

#endif
