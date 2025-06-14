struc ts_process
    .m_id: resw 1
    .m_parentSegment: resw 1
    .m_parentOffset: resw 1
    .m_threadListSegment: resw 1
    .m_threadListOffset: resw 1
    .m_memoryAllocationListSegment: resw 1
    .m_memoryAllocationListOffset: resw 1
    .m_codeSegment: resw 1
endstruc

; struct ts_process *process_new(struct ts_process *p_parent)
process_new:
    %define p_parentSegment (bp + 4)
    %define p_parentOffset (bp + 6)

    push bp
    mov bp, sp

    mov ax, ts_process_size
    push ax
    call malloc
    add sp, 2

    ; If malloc() failed, return NULL.
    test dx, dx
    jz .end

    push es
    push di
    push dx
    push ax
    mov es, dx
    mov di, ax

    ; Set the PID
    call criticalSection_enter

    mov ax, [g_process_nextPid]
    inc word [g_process_nextPid]
    
    call criticalSection_leave

    mov es:[di + ts_process.m_id], ax

    ; Set the parent
    mov ax, [p_parentSegment]
    mov es:[di + ts_process.m_parentSegment], ax
    mov ax, [p_parentOffset]
    mov es:[di + ts_process.m_parentOffset], ax

    ; Initialize the thread list
    xor ax, ax
    mov es:[di + ts_process.m_threadListSegment], ax
    mov es:[di + ts_process.m_threadListOffset], ax

    ; Initialize the memory allocation list
    mov es:[di + ts_process.m_memoryAllocationListSegment], ax
    mov es:[di + ts_process.m_memoryAllocationListOffset], ax

    pop ax
    pop dx
    pop di
    pop es

.end:
    pop bp
    ret
    
    %undef p_parentSegment
    %undef p_parentOffset

section .data:
g_process_nextPid: dw 1
