#include "printk.h"
#include "task.h"

void isr_exception(uint64_t p_exception, uint64_t p_errorCode) {
    pr_crit("========== Kernel panic ==========\n");
    pr_crit("Exception: %lu\n", p_exception);
    pr_crit("Error code: 0x%016lx\n", p_errorCode);
    pr_crit("RAX=0x%016lx\n", g_task_currentTaskContext.m_rax);
    pr_crit("RBX=0x%016lx\n", g_task_currentTaskContext.m_rbx);
    pr_crit("RCX=0x%016lx\n", g_task_currentTaskContext.m_rcx);
    pr_crit("RDX=0x%016lx\n", g_task_currentTaskContext.m_rdx);
    pr_crit("RBP=0x%016lx\n", g_task_currentTaskContext.m_rbp);
    pr_crit("RSP=0x%016lx\n", g_task_currentTaskContext.m_rsp);
    pr_crit("RSI=0x%016lx\n", g_task_currentTaskContext.m_rsi);
    pr_crit("RDI=0x%016lx\n", g_task_currentTaskContext.m_rdi);
    pr_crit("RIP=0x%016lx\n", g_task_currentTaskContext.m_rip);
    pr_crit("RFLAGS=0x%016lx\n", g_task_currentTaskContext.m_rflags);
    pr_crit("R8=0x%016lx\n", g_task_currentTaskContext.m_r8);
    pr_crit("R9=0x%016lx\n", g_task_currentTaskContext.m_r9);
    pr_crit("R10=0x%016lx\n", g_task_currentTaskContext.m_r10);
    pr_crit("R11=0x%016lx\n", g_task_currentTaskContext.m_r11);
    pr_crit("R12=0x%016lx\n", g_task_currentTaskContext.m_r12);
    pr_crit("R13=0x%016lx\n", g_task_currentTaskContext.m_r13);
    pr_crit("R14=0x%016lx\n", g_task_currentTaskContext.m_r14);
    pr_crit("R15=0x%016lx\n", g_task_currentTaskContext.m_r15);
    pr_crit("CS=0x%014lx\n", g_task_currentTaskContext.m_cs);
    pr_crit("DS=0x%014lx\n", g_task_currentTaskContext.m_ds);
    pr_crit("ES=0x%014lx\n", g_task_currentTaskContext.m_es);
    pr_crit("FS=0x%014lx\n", g_task_currentTaskContext.m_fs);
    pr_crit("GS=0x%014lx\n", g_task_currentTaskContext.m_gs);
    pr_crit("SS=0x%014lx\n", g_task_currentTaskContext.m_ss);
    pr_crit("==================================\n");
    pr_crit("System halted.");

    while(1) {
        asm("cli");
        asm("hlt");
    }
}
