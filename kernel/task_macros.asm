%macro M_TASK_SAVE_CONTEXT 0
    ; Set DS to the kernel segment
    push ds
    push ax
    push bp
    mov ax, cs
    mov ds, ax

    ; Prepare access to the stack
    mov bp, sp
    add bp, 6

    ; Stack layout:
    ; - [bp - 6]: bp
    ; - [bp - 4]: ax
    ; - [bp - 2]: ds
    ; - [bp]: ip
    ; - [bp + 2]: cs
    ; - [bp + 4]: flags

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
    lea ax, [bp + 6] ; AX = SP before interrupt
    mov [g_task_currentTaskContext + ts_taskContext.m_sp], ax
    mov [g_task_currentTaskContext + ts_taskContext.m_si], si
    mov [g_task_currentTaskContext + ts_taskContext.m_di], di
    mov [g_task_currentTaskContext + ts_taskContext.m_es], es
    mov [g_task_currentTaskContext + ts_taskContext.m_ss], ss
    mov ax, [bp]
    mov [g_task_currentTaskContext + ts_taskContext.m_ip], ax
    mov ax, [bp + 2]
    mov [g_task_currentTaskContext + ts_taskContext.m_cs], ax
    mov ax, [bp + 4]
    mov [g_task_currentTaskContext + ts_taskContext.m_flags], ax

    ; Restore DS
    pop ds
%endmacro
