#ifndef __INCLUDE_THREAD_H__
#define __INCLUDE_THREAD_H__

#include "process.h"
#include "task.h"

enum te_thread_state {
    E_THREAD_STATE_INITIALIZING,
    E_THREAD_STATE_READY,
    E_THREAD_STATE_RUNNING,
    E_THREAD_STATE_SUSPENDED
};

struct ts_thread {
    enum te_thread_state m_state;
    struct ts_process *m_process;
    struct ts_memoryRange_listNode *m_memoryRangeList;
    void *m_tdata;
    void *m_tbss;
    struct ts_task_context m_context;
};

struct ts_thread *thread_new(struct ts_process *p_process, size_t p_stackSize);
void thread_destroy(struct ts_thread *p_thread);
void *thread_allocPages(struct ts_thread *p_thread, size_t p_size);

#endif
