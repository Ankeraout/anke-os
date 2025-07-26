#ifndef __INCLUDE_PROCESS_H__
#define __INCLUDE_PROCESS_H__

#include "memoryRange.h"
#include "mm/vmm.h"
#include "sys/types.h"

struct ts_process {
    pid_t m_pid;
    struct ts_process *m_parent;
    struct ts_memoryRange_listNode *m_memoryRangeList;
    struct ts_vmm_context *m_memoryContext;
    struct ts_list_node *m_threadList;
    struct ts_list_node *m_childList;
    void *m_tdata;
    size_t m_tdataSize;
    size_t m_tbssSize;
};

struct ts_process *process_new(
    struct ts_process *p_parent,
    void *p_tdata,
    size_t m_tdataSize,
    size_t m_tbssSize
);
void process_destroy(struct ts_process *p_process);

#endif
