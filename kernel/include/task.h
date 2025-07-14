#ifndef __INCLUDE_TASK_H__
#define __INCLUDE_TASK_H__

#include <stdint.h>

struct ts_taskContext {
    uint64_t m_rax;
    uint64_t m_rbx;
    uint64_t m_rcx;
    uint64_t m_rdx;
    uint64_t m_rbp;
    uint64_t m_rsp;
    uint64_t m_rsi;
    uint64_t m_rdi;
    uint64_t m_r8;
    uint64_t m_r9;
    uint64_t m_r10;
    uint64_t m_r11;
    uint64_t m_r12;
    uint64_t m_r13;
    uint64_t m_r14;
    uint64_t m_r15;
    uint64_t m_rip;
    uint64_t m_cs;
    uint64_t m_ds;
    uint64_t m_es;
    uint64_t m_fs;
    uint64_t m_gs;
    uint64_t m_ss;
    uint64_t m_rflags;
};

void task_save(struct ts_taskContext *p_context);
void task_load(struct ts_taskContext *p_context);
void task_resume(void);

extern struct ts_taskContext g_task_currentTaskContext;

#endif
