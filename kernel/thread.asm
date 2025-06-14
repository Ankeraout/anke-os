%define E_THREADSTATUS_READY 0
%define E_THREADSTATUS_RUNNING 1
%define E_THREADSTATUS_SUSPENDED 2
%define E_THREADSTATUS_WAITING 3

struc ts_thread
    .m_status: resw 1
    .m_taskSegment: resw 1
    .m_taskOffset: resw 1
    .m_memoryAllocationListSegment: resw 1
    .m_memoryAllocationListOffset: resw 1
endstruc

section .text

; struct ts_thread *thread_new(
;     struct ts_process *p_process,
;     uint16_t p_codeSegment,
;     uint16_t p_codeOffset,
;     uint16_t p_stackSize
; )
thread_new:
    %define p_processSegment (bp + 4)
    %define p_processOffset (bp + 6)
    %define p_codeSegment (bp + 8)
    %define p_codeOffset (bp + 10)
    %define p_stackSize (bp + 12)
    %define l_stackSegment (bp - 4)
    %define l_stackOffset (bp - 2)

    push bp
    mov bp, sp

    sub sp, 4

    ; Save ES and DI for later
    push es
    push di

    ; Make ES:DI point to the process
    mov ax, [p_processOffset]
    mov dx, [p_processSegment]

    ; Allocate struct ts_thread
    mov ax, ts_thread_size
    push ax
    call malloc
    add sp, 2

    ; If malloc returned NULL, then return NULL.
    cmp dx, 0
    jz .end

    ; Make ES:DI point to the new struct ts_thread.
    mov es, dx
    mov di, ax

    ; Allocate stack
    mov ax, [p_stackSize]
    push ax
    call malloc
    add sp, 2

    ; If malloc returned NULL, then return NULL.
    cmp dx, 0
    jz .failedToAllocateStack

    ; Save the stack segment and offset for later.
    mov [l_stackSegment], dx
    mov [l_stackOffset], ax

    ; Set the thread state
    mov word es:[di + ts_thread.m_status], E_THREADSTATUS_READY

    ; Create a new task
    push di
    push es
    push word [p_processOffset]
    push word [p_processSegment]
    call task_new
    add sp, 8

    ; If task_new() returned NULL, abort.
    cmp dx, 0
    jz .failedToCreateTask

    ; Initialize task context
    mov es:[di + ts_thread.m_taskSegment], dx
    mov es:[di + ts_thread.m_taskOffset], ax

    push es
    push di

    mov es, dx
    mov di, ax

    mov ax, [l_stackSegment]
    mov es:[di + ts_task.m_context + ts_taskContext.m_ss], ax
    mov ax, [l_stackOffset]
    add ax, [p_stackSize]
    mov es:[di + ts_task.m_context + ts_taskContext.m_sp], ax
    mov word es:[di + ts_task.m_context + ts_taskContext.m_flags], 0x0200
    mov ax, [p_codeSegment]
    mov es:[di + ts_task.m_context + ts_taskContext.m_cs], ax
    mov es:[di + ts_task.m_context + ts_taskContext.m_ds], ax
    mov es:[di + ts_task.m_context + ts_taskContext.m_es], ax
    mov ax, [p_codeOffset]
    mov es:[di + ts_task.m_context + ts_taskContext.m_ip], ax

    pop ax
    pop dx

.end:
    pop di
    pop es
    add sp, 4
    pop bp
    ret
    
.failedToCreateTask:
    push word [l_stackOffset]
    push word [l_stackSegment]
    call free
    add sp, 4

.failedToAllocateStack:
    ; Free the allocated struct ts_thread
    push di
    push es
    call free
    add sp, 4
    jmp .end

    %undef p_processSegment
    %undef p_processOffset
    %undef p_codeOffset
    %undef p_stackSize
    %undef l_codeSegment
    %undef l_stackSegment
    %undef l_stackOffset
