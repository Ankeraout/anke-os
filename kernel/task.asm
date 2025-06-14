%macro M_TASK_SAVE_CONTEXT 0
    ; Set DS to the kernel segment
    push ds
    push ax
    push bp
    mov ax, C_KERNEL_SEGMENT
    mov ds, ax

    ; Prepare access to the stack
    mov bp, sp
    add bp, 6

    ; Save the value of the registers
    pop ax
    mov [g_task_currentTaskContext + ts_taskContext.m_bp], ax
    pop ax
    mov [g_task_currentTaskContext + ts_taskContext.m_ax], ax
    mov ax, [bp - 2]
    mov [g_task_currentTaskContext + ts_taskContext.m_ds], ax
    mov [g_task_currentTaskContext + ts_taskContext.m_bx], bx
    mov [g_task_currentTaskContext + ts_taskContext.m_cx], cx
    mov [g_task_currentTaskContext + ts_taskContext.m_dx], dx
    lea ax, [bp + 6]
    mov [g_task_currentTaskContext + ts_taskContext.m_sp], ax
    mov [g_task_currentTaskContext + ts_taskContext.m_si], si
    mov [g_task_currentTaskContext + ts_taskContext.m_di], di
    mov ax, es
    mov [g_task_currentTaskContext + ts_taskContext.m_es], ax
    mov ax, ss
    mov [g_task_currentTaskContext + ts_taskContext.m_ss], ax
    mov ax, [bp]
    mov [g_task_currentTaskContext + ts_taskContext.m_ip], ax
    mov ax, [bp + 2]
    mov [g_task_currentTaskContext + ts_taskContext.m_cs], ax
    mov ax, [bp + 4]
    mov [g_task_currentTaskContext + ts_taskContext.m_flags], ax

    ; Restore DS
    pop ds
%endmacro

struc ts_taskContext
    .m_ax: resw 1
    .m_bx: resw 1
    .m_cx: resw 1
    .m_dx: resw 1
    .m_sp: resw 1
    .m_bp: resw 1
    .m_si: resw 1
    .m_di: resw 1
    .m_ds: resw 1
    .m_es: resw 1
    .m_ss: resw 1
    .m_cs: resw 1
    .m_ip: resw 1
    .m_flags: resw 1
endstruc

struc ts_task
    .m_processSegment: resw 1
    .m_processOffset: resw 1
    .m_threadSegment: resw 1
    .m_threadOffset: resw 1
    .m_context: resb ts_taskContext_size
endstruc

section .text

; struct ts_task *task_new(
;     struct ts_process *p_process,
;     struct ts_thread *p_thread
; )
task_new:
    %define p_processSegment (bp + 4)
    %define p_processOffset (bp + 6)
    %define p_threadSegment (bp + 8)
    %define p_threadOffset (bp + 10)

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
    mov ax, ts_taskContext_size
    push ax
    mov ax, g_task_currentTaskContext
    push ax
    push cs
    mov ax, [g_task_currentTaskOffset]
    add ax, ts_task.m_context
    push ax
    push word [g_task_currentTaskSegment]
    call memcpy
    add sp, 10

    ret

; void task_load(struct ts_task *p_task)
task_load:
    %define p_taskSegment (bp + 4)
    %define p_taskOffset (bp + 6)

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
    mov ax, [g_task_currentTaskOffset]
    add ax, ts_task.m_context
    push ax
    push word [g_task_currentTaskSegment]
    mov ax, g_task_currentTaskContext
    push ax
    push cs
    call memcpy
    add sp, 10

    pop bp
    ret

    %undef p_taskSegment
    %undef p_taskOffset

; void task_resume()
task_resume:
    ; Set the correct register values
    mov ax, [g_task_currentTaskContext + ts_taskContext.m_ss]
    mov ss, ax
    mov ax, [g_task_currentTaskContext + ts_taskContext.m_es]
    mov es, ax
    mov di, [g_task_currentTaskContext + ts_taskContext.m_di]
    mov si, [g_task_currentTaskContext + ts_taskContext.m_si]
    mov bp, [g_task_currentTaskContext + ts_taskContext.m_bp]
    mov sp, [g_task_currentTaskContext + ts_taskContext.m_sp]
    mov dx, [g_task_currentTaskContext + ts_taskContext.m_dx]
    mov cx, [g_task_currentTaskContext + ts_taskContext.m_cx]
    mov bx, [g_task_currentTaskContext + ts_taskContext.m_bx]

    ; Prepare the stack for the IRET opcode
    push word [g_task_currentTaskContext + ts_taskContext.m_flags]
    push word [g_task_currentTaskContext + ts_taskContext.m_cs]
    push word [g_task_currentTaskContext + ts_taskContext.m_ip]

    ; Set DS and AX
    push word [g_task_currentTaskContext + ts_taskContext.m_ds]
    mov ax, [g_task_currentTaskContext + ts_taskContext.m_ax]
    pop ds

    ; Jump to the task
    iret

section .bss
g_task_currentTaskSegment: resw 1
g_task_currentTaskOffset: resw 1
g_task_currentTaskContext: resb ts_taskContext_size
