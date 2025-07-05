struc ts_taskContext
    .m_ax: resw 1
    .m_bx: resw 1
    .m_cx: resw 1
    .m_dx: resw 1
    .m_bp: resw 1
    .m_si: resw 1
    .m_ds: resw 1
    .m_di: resw 1
    .m_es: resw 1
    .m_sp: resw 1
    .m_ss: resw 1
    .m_ip: resw 1
    .m_cs: resw 1
    .m_flags: resw 1
endstruc

struc ts_task
    .m_processOffset: resw 1
    .m_processSegment: resw 1
    .m_threadOffset: resw 1
    .m_threadSegment: resw 1
    .m_context: resb ts_taskContext_size
endstruc

section .text

; struct ts_task *task_new(
;     struct ts_process *p_process,
;     struct ts_thread *p_thread
; )
task_new:
    %define p_processOffset (bp + 4)
    %define p_processSegment (bp + 6)
    %define p_threadOffset (bp + 8)
    %define p_threadSegment (bp + 10)

    push bp
    mov bp, sp

    ; Allocate memory for the task object
    mov ax, ts_task_size
    push ax
    call malloc
    add sp, 2

    ; If malloc() returned NULL, return NULL.
    cmp dx, 0
    jz .end

    ; Make ES:DI point to the task object
    push es
    push di
    mov es, dx
    mov di, ax

    ; Initialize the task object
    mov ax, [p_processSegment]
    mov es:[di + ts_task.m_processSegment], ax
    mov ax, [p_processOffset]
    mov es:[di + ts_task.m_processOffset], ax
    mov ax, [p_threadSegment]
    mov es:[di + ts_task.m_threadSegment], ax
    mov ax, [p_threadOffset]
    mov es:[di + ts_task.m_threadOffset], ax

    ; Register the task object
    push es
    push di
    call scheduler_add
    add sp, 4

    mov dx, es
    mov ax, di

    pop di
    pop es

.end:
    pop bp
    ret

    %undef p_processSegment
    %undef p_processOffset
    %undef p_threadSegment
    %undef p_threadOffset

; void task_save()
task_save:
    ; If there is no task to save, do not save.
    mov ax, [g_task_currentTaskSegment]
    or ax, [g_task_currentTaskOffset]
    jz .end

    mov ax, ts_taskContext_size
    push ax
    mov ax, g_task_currentTaskContext
    push cs
    push ax
    push word [g_task_currentTaskSegment]
    mov ax, [g_task_currentTaskOffset]
    add ax, ts_task.m_context
    push ax
    call memcpy
    add sp, 10

    .end:
        ret

; void task_load(struct ts_task *p_task)
task_load:
    %define p_taskOffset (bp + 4)
    %define p_taskSegment (bp + 6)

    push bp
    mov bp, sp

    ; Set the current task
    mov ax, [p_taskSegment]
    mov [g_task_currentTaskSegment], ax
    mov ax, [p_taskOffset]
    mov [g_task_currentTaskOffset], ax

    ; Load the task context
    mov ax, ts_taskContext_size
    push ax
    push word [g_task_currentTaskSegment]
    mov ax, [g_task_currentTaskOffset]
    add ax, ts_task.m_context
    push ax
    push cs
    mov ax, g_task_currentTaskContext
    push ax
    call memcpy
    add sp, 10

    pop bp
    ret

    %undef p_taskSegment
    %undef p_taskOffset

; void task_resume()
task_resume:
    ; Prepare the stack for the IRET opcode
    mov ss, [g_task_currentTaskContext + ts_taskContext.m_ss]
    mov sp, [g_task_currentTaskContext + ts_taskContext.m_sp]
    push word [g_task_currentTaskContext + ts_taskContext.m_flags]
    push word [g_task_currentTaskContext + ts_taskContext.m_cs]
    push word [g_task_currentTaskContext + ts_taskContext.m_ip]

    ; Set the correct register values
    les di, [g_task_currentTaskContext + ts_taskContext.m_di]
    mov bp, [g_task_currentTaskContext + ts_taskContext.m_bp]
    mov dx, [g_task_currentTaskContext + ts_taskContext.m_dx]
    mov cx, [g_task_currentTaskContext + ts_taskContext.m_cx]
    mov bx, [g_task_currentTaskContext + ts_taskContext.m_bx]
    mov ax, [g_task_currentTaskContext + ts_taskContext.m_ax]
    lds si, [g_task_currentTaskContext + ts_taskContext.m_si]

    ; Jump to the task
    iret

; void task_unload()
task_unload:
    call task_save

; void task_clear()
task_clear:
    xor ax, ax
    mov word [g_task_currentTaskSegment], ax
    mov word [g_task_currentTaskOffset], ax
    ret

; void task_destroy(struct ts_task *p_task)
task_destroy:
    %define p_taskOffset (bp + 4)
    %define p_taskSegment (bp + 6)

    push bp
    mov bp, sp

    .unscheduleTask:
        push word [p_taskSegment]
        push word [p_taskOffset]
        call scheduler_remove
        add sp, 4

    .destroyTask:
        push word [p_taskSegment]
        push word [p_taskOffset]
        call free
        add sp, 4
    
    .end:
        pop bp
        ret

    %undef p_taskSegment
    %undef p_taskOffset

section .bss
align 2, resb 1
g_task_currentTaskOffset: resw 1
g_task_currentTaskSegment: resw 1
g_task_currentTaskContext: resb ts_taskContext_size
