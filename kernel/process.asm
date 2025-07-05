struc ts_process
    .m_id: resw 1
    .m_parentOffset: resw 1
    .m_parentSegment: resw 1
    .m_threadListOffset: resw 1
    .m_threadListSegment: resw 1
    .m_memoryAllocationListOffset: resw 1
    .m_memoryAllocationListSegment: resw 1
    .m_codeSegment: resw 1
    .m_childListOffset: resw 1
    .m_childListSegment: resw 1
endstruc

section .text

; struct ts_process *process_new(struct ts_process *p_parent)
process_new:
    %define p_parentOffset (bp + 4)
    %define p_parentSegment (bp + 6)

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

    ; Initialize the child list
    mov es:[di + ts_process.m_childListSegment], ax
    mov es:[di + ts_process.m_childListOffset], ax

    pop ax
    pop dx
    pop di
    pop es

.end:
    pop bp
    ret
    
    %undef p_parentSegment
    %undef p_parentOffset

; void *process_destroy(struct ts_process *p_process)
process_destroy:
    %define p_processOffset (bp + 4)
    %define p_processSegment (bp + 6)

    push bp
    mov bp, sp

    ; Destroy threads
    .initThreadDestroyLoop:
        les di, [p_processOffset]
        les di, es:[di + ts_process.m_threadListOffset]

    .threadDestroyLoop:
        mov ax, es
        or ax, di
        jz .threadDestroyLoopEnd

        push word es:[di + ts_listElement.m_dataSegment]
        push word es:[di + ts_listElement.m_dataOffset]
        call thread_destroy
        add sp, 4

        push es
        push di
        les di, es:[di + ts_listElement.m_nextOffset]
        call free
        add sp, 4

        jmp .threadDestroyLoop

    .threadDestroyLoopEnd:
        
    .initMemoryAllocationDestroyLoop:
        les di, [p_processOffset]
        les di, es:[di + ts_process.m_memoryAllocationListOffset]

    .memoryAllocationDestroyLoop:
        mov ax, es
        or ax, di
        jz .memoryAllocationDestroyLoopEnd

        push es
        push di

        les di, es:[di + ts_listElement.m_dataOffset]

        push es
        push di

        ; Free memory
        xor ax, ax
        push ax
        push word es:[di + ts_memoryAllocation.m_segmentCount]
        push word es:[di + ts_memoryAllocation.m_segment]
        call mm_mark
        add sp, 6

        ; Free memory allocation
        call free
        add sp, 4

        ; Free list element
        pop di
        pop es
        les di, es:[di + ts_listElement.m_nextOffset]
        sub sp, 4
        call free
        add sp, 4

        jmp .memoryAllocationDestroyLoop

    .memoryAllocationDestroyLoopEnd:

    .removeFromParentSubprocessList:
        les di, [p_processOffset]
        les di, es:[di + ts_process.m_parentOffset]
        lea ax, [di + ts_process.m_childListOffset]
        push word [p_processSegment]
        push word [p_processOffset]
        push es
        push ax
        call list_remove
        add sp, 8

    .destroyProcess:
        push word [p_processOffset]
        push word [p_processSegment]
        call free
        add sp, 4

    .end:
        pop bp
        ret

    %undef p_processOffset
    %undef p_processSegment

section .data
align 2
g_process_nextPid: dw 1
