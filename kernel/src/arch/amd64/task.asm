bits 64

global task_resume
extern g_task_currentTaskContext

%include "arch/amd64/task.inc"

section .text

task_resume:
    mov rax, [g_task_currentTaskContext + ts_task_context.m_rax]
    mov rbx, [g_task_currentTaskContext + ts_task_context.m_rbx]
    mov rcx, [g_task_currentTaskContext + ts_task_context.m_rcx]
    mov rdx, [g_task_currentTaskContext + ts_task_context.m_rdx]
    mov rbp, [g_task_currentTaskContext + ts_task_context.m_rbp]
    mov rsi, [g_task_currentTaskContext + ts_task_context.m_rsi]
    mov rdi, [g_task_currentTaskContext + ts_task_context.m_rdi]
    mov r8, [g_task_currentTaskContext + ts_task_context.m_r8]
    mov r9, [g_task_currentTaskContext + ts_task_context.m_r9]
    mov r10, [g_task_currentTaskContext + ts_task_context.m_r10]
    mov r11, [g_task_currentTaskContext + ts_task_context.m_r11]
    mov r12, [g_task_currentTaskContext + ts_task_context.m_r12]
    mov r13, [g_task_currentTaskContext + ts_task_context.m_r13]
    mov r14, [g_task_currentTaskContext + ts_task_context.m_r14]
    mov r15, [g_task_currentTaskContext + ts_task_context.m_r15]
    mov ds, [g_task_currentTaskContext + ts_task_context.m_ds]
    mov es, [g_task_currentTaskContext + ts_task_context.m_es]
    mov fs, [g_task_currentTaskContext + ts_task_context.m_fs]
    mov gs, [g_task_currentTaskContext + ts_task_context.m_gs]
    push qword [g_task_currentTaskContext + ts_task_context.m_ss]
    push qword [g_task_currentTaskContext + ts_task_context.m_rsp]
    push qword [g_task_currentTaskContext + ts_task_context.m_rflags]
    push qword [g_task_currentTaskContext + ts_task_context.m_cs]
    push qword [g_task_currentTaskContext + ts_task_context.m_rip]
    iretq
