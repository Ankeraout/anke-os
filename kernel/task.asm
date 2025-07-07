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

section .text

; void task_save(struct ts_taskContext *p_context)
task_save:
    %define p_contextOffset (bp + 4)
    %define p_contextSegment (bp + 6)
    
    push bp
    mov bp, sp

    mov ax, ts_taskContext_size
    push ax
    push cs
    mov ax, g_task_currentTaskContext
    push ax
    push word [p_contextSegment]
    push word [p_contextOffset]
    call memcpy
    add sp, 10

    pop bp
    ret

    %undef p_contextOffset
    %undef p_contextSegment

; void task_load(struct ts_taskContext *p_context)
task_load:
    %define p_contextOffset (bp + 4)
    %define p_contextSegment (bp + 6)

    push bp
    mov bp, sp

    mov ax, ts_taskContext_size
    push ax
    push word [p_contextSegment]
    push word [p_contextOffset]
    push cs
    mov ax, g_task_currentTaskContext
    push ax
    call memcpy
    add sp, 10

    pop bp
    ret

    %undef p_contextSegment
    %undef p_contextOffset

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

section .bss
align 2, resb 1
g_task_currentTaskContext: resb ts_taskContext_size
