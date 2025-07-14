bits 64

%include "arch/amd64/task.inc"

%macro M_DEFINE_EXCEPTION 1
    global isrException%1
    isrException%1:
        push 0 ; No error code
        push %1 ; Interrupt number
        jmp isrExceptionCommon
%endmacro

%macro M_DEFINE_EXCEPTION_ERRCODE 1
    global isrException%1
    isrException%1:
        push %1 ; Interrupt number
        jmp isrExceptionCommon
%endmacro

%macro M_DEFINE_IRQ 1
    global isrIrq%1
    isrIrq%1:
        push %1 ; Interrupt number
        jmp isrIrqCommon
%endmacro

extern irq_service
extern isr_exception
extern g_task_currentTaskContext

section .text
M_DEFINE_EXCEPTION 0
M_DEFINE_EXCEPTION 1
M_DEFINE_EXCEPTION 2
M_DEFINE_EXCEPTION 3
M_DEFINE_EXCEPTION 4
M_DEFINE_EXCEPTION 5
M_DEFINE_EXCEPTION 6
M_DEFINE_EXCEPTION 7
M_DEFINE_EXCEPTION_ERRCODE 8
M_DEFINE_EXCEPTION 9
M_DEFINE_EXCEPTION_ERRCODE 10
M_DEFINE_EXCEPTION_ERRCODE 11
M_DEFINE_EXCEPTION_ERRCODE 12
M_DEFINE_EXCEPTION_ERRCODE 13
M_DEFINE_EXCEPTION_ERRCODE 14
M_DEFINE_EXCEPTION 15
M_DEFINE_EXCEPTION 16
M_DEFINE_EXCEPTION_ERRCODE 17
M_DEFINE_EXCEPTION 18
M_DEFINE_EXCEPTION 19
M_DEFINE_EXCEPTION 20
M_DEFINE_EXCEPTION_ERRCODE 21
M_DEFINE_EXCEPTION 22
M_DEFINE_EXCEPTION 23
M_DEFINE_EXCEPTION 24
M_DEFINE_EXCEPTION 25
M_DEFINE_EXCEPTION 26
M_DEFINE_EXCEPTION 27
M_DEFINE_EXCEPTION 28
M_DEFINE_EXCEPTION_ERRCODE 29
M_DEFINE_EXCEPTION_ERRCODE 30
M_DEFINE_EXCEPTION 31
M_DEFINE_IRQ 0
M_DEFINE_IRQ 1
M_DEFINE_IRQ 2
M_DEFINE_IRQ 3
M_DEFINE_IRQ 4
M_DEFINE_IRQ 5
M_DEFINE_IRQ 6
M_DEFINE_IRQ 7
M_DEFINE_IRQ 8
M_DEFINE_IRQ 9
M_DEFINE_IRQ 10
M_DEFINE_IRQ 11
M_DEFINE_IRQ 12
M_DEFINE_IRQ 13
M_DEFINE_IRQ 14
M_DEFINE_IRQ 15

isrExceptionCommon:; Set the value of DS
    push rax
    mov ax, ds
    push rax
    mov ax, 0x10
    mov ds, ax
    
    ; Save the context
    pop qword [g_task_currentTaskContext + ts_taskContext.m_ds]
    pop qword [g_task_currentTaskContext + ts_taskContext.m_rax]
    mov [g_task_currentTaskContext + ts_taskContext.m_rbx], rbx
    mov [g_task_currentTaskContext + ts_taskContext.m_rcx], rcx
    mov [g_task_currentTaskContext + ts_taskContext.m_rdx], rdx
    mov [g_task_currentTaskContext + ts_taskContext.m_rbp], rbp
    mov [g_task_currentTaskContext + ts_taskContext.m_rsi], rsi
    mov [g_task_currentTaskContext + ts_taskContext.m_rdi], rdi
    mov [g_task_currentTaskContext + ts_taskContext.m_r8], r8
    mov [g_task_currentTaskContext + ts_taskContext.m_r9], r9
    mov [g_task_currentTaskContext + ts_taskContext.m_r10], r10
    mov [g_task_currentTaskContext + ts_taskContext.m_r11], r11
    mov [g_task_currentTaskContext + ts_taskContext.m_r12], r12
    mov [g_task_currentTaskContext + ts_taskContext.m_r13], r13
    mov [g_task_currentTaskContext + ts_taskContext.m_r14], r14
    mov [g_task_currentTaskContext + ts_taskContext.m_r15], r15
    mov [g_task_currentTaskContext + ts_taskContext.m_es], es
    mov [g_task_currentTaskContext + ts_taskContext.m_fs], fs
    mov [g_task_currentTaskContext + ts_taskContext.m_gs], gs
    
    ; Exception number
    pop rdi

    ; Exception code
    pop rsi

    ; IRETQ stack layout
    pop qword [g_task_currentTaskContext + ts_taskContext.m_rip]
    pop qword [g_task_currentTaskContext + ts_taskContext.m_cs]
    pop qword [g_task_currentTaskContext + ts_taskContext.m_rflags]
    pop qword [g_task_currentTaskContext + ts_taskContext.m_rsp]
    pop qword [g_task_currentTaskContext + ts_taskContext.m_ss]

    jmp isr_exception

isrIrqCommon:
    ; Set the value of DS
    push rax
    mov ax, ds
    push rax
    mov ax, 0x10
    mov ds, ax
    
    ; Save the context
    pop qword [g_task_currentTaskContext + ts_taskContext.m_ds]
    pop qword [g_task_currentTaskContext + ts_taskContext.m_rax]
    mov [g_task_currentTaskContext + ts_taskContext.m_rbx], rbx
    mov [g_task_currentTaskContext + ts_taskContext.m_rcx], rcx
    mov [g_task_currentTaskContext + ts_taskContext.m_rdx], rdx
    mov [g_task_currentTaskContext + ts_taskContext.m_rbp], rbp
    mov [g_task_currentTaskContext + ts_taskContext.m_rsi], rsi
    mov [g_task_currentTaskContext + ts_taskContext.m_rdi], rdi
    mov [g_task_currentTaskContext + ts_taskContext.m_r8], r8
    mov [g_task_currentTaskContext + ts_taskContext.m_r9], r9
    mov [g_task_currentTaskContext + ts_taskContext.m_r10], r10
    mov [g_task_currentTaskContext + ts_taskContext.m_r11], r11
    mov [g_task_currentTaskContext + ts_taskContext.m_r12], r12
    mov [g_task_currentTaskContext + ts_taskContext.m_r13], r13
    mov [g_task_currentTaskContext + ts_taskContext.m_r14], r14
    mov [g_task_currentTaskContext + ts_taskContext.m_r15], r15
    mov [g_task_currentTaskContext + ts_taskContext.m_es], es
    mov [g_task_currentTaskContext + ts_taskContext.m_fs], fs
    mov [g_task_currentTaskContext + ts_taskContext.m_gs], gs
    
    ; IRQ number
    pop rdi

    ; IRETQ stack layout
    pop qword [g_task_currentTaskContext + ts_taskContext.m_rip]
    pop qword [g_task_currentTaskContext + ts_taskContext.m_cs]
    pop qword [g_task_currentTaskContext + ts_taskContext.m_rflags]
    pop qword [g_task_currentTaskContext + ts_taskContext.m_rsp]
    pop qword [g_task_currentTaskContext + ts_taskContext.m_ss]

    jmp irq_service
