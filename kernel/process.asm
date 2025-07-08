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
    %define l_processSegment (bp - 2)
    %define l_processOffset (bp - 4)
    %define l_listElementSegment (bp - 6)
    %define l_listElementOffset (bp - 8)

    push bp
    mov bp, sp
    sub sp, 4
    push es
    push di

    call criticalSection_enter

    .allocateProcess:
        mov ax, ts_process_size
        push ax
        call malloc
        add sp, 2

        mov cx, ax
        or cx, dx
        jz .end

        mov [l_processSegment], dx
        mov [l_processOffset], ax

    .allocateListElement:
        mov ax, ts_listElement_size
        push ax
        call malloc
        add sp, 2

        mov cx, ax
        or cx, dx
        jz .failedToAllocateListElement

        mov [l_listElementSegment], dx
        mov [l_listElementOffset], ax

    .initializeListElement:
        les di, [p_parentOffset]
        xchg ax, es:[di + ts_process.m_childListOffset]
        xchg dx, es:[di + ts_process.m_childListSegment]
        les di, [l_listElementOffset]
        mov es:[di + ts_listElement.m_nextOffset], ax
        mov es:[di + ts_listElement.m_nextSegment], dx
        mov ax, [l_processOffset]
        mov es:[di + ts_listElement.m_dataOffset], ax
        mov ax, [l_processSegment]
        mov es:[di + ts_listElement.m_dataSegment], ax

    .setProcessId:
        mov ax, [g_process_nextPid]
        inc word [g_process_nextPid]
        les di, [l_processOffset]
        mov es:[di + ts_process.m_id], ax

    .setParentProcess:
        mov ax, [p_parentOffset]
        mov es:[di + ts_process.m_parentOffset], ax
        mov ax, [p_parentSegment]
        mov es:[di + ts_process.m_parentSegment], ax

    .initializeThreadList:
        xor ax, ax
        mov es:[di + ts_process.m_threadListSegment], ax
        mov es:[di + ts_process.m_threadListOffset], ax

    .initializeMemoryAllocationList:
        mov es:[di + ts_process.m_memoryAllocationListSegment], ax
        mov es:[di + ts_process.m_memoryAllocationListOffset], ax

    .initializeChildList:
        mov es:[di + ts_process.m_childListSegment], ax
        mov es:[di + ts_process.m_childListOffset], ax

    .end:
        call criticalSection_leave
        mov ax, [l_processOffset]
        mov dx, [l_processSegment]
        pop di
        pop es
        add sp, 4
        pop bp
        ret

    .failedToAllocateListElement:
        push word [l_processSegment]
        push word [l_processOffset]
        call free
        add sp, 4
        xor ax, ax
        mov [l_processSegment], ax
        mov [l_processOffset], ax
        jmp .end
    
    %undef p_parentSegment
    %undef p_parentOffset

; void *process_destroy(struct ts_process *p_process)
process_destroy:
    %define p_processOffset (bp + 4)
    %define p_processSegment (bp + 6)

    push bp
    mov bp, sp

    call criticalSection_enter

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
        call criticalSection_leave
        pop bp
        ret

    %undef p_processOffset
    %undef p_processSegment

section .data
align 2
g_process_nextPid: dw 1
