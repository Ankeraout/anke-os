#ifndef __INCLUDE_THREAD_H__
#define __INCLUDE_THREAD_H__

#include <stddef.h>

#include "list.h"
#include "process.h"
#include "task.h"

enum te_thread_status {
    E_THREADSTATUS_INITIALIZING,
    E_THREADSTATUS_READY,
    E_THREADSTATUS_RUNNING,
    E_THREADSTATUS_SUSPENDED,
    E_THREADSTATUS_WAITING
};

struct ts_thread {
    enum te_thread_status m_status;
    struct ts_list_node *m_memoryAllocationList;
    struct ts_task_context m_context;
};

struct ts_thread *thread_new(
    struct ts_process *p_process,
    void *p_entryPoint,
    size_t p_stackSize
);
void thread_start(struct ts_thread *p_thread);
void thread_destroy(struct ts_thread *p_thread);

#endif
