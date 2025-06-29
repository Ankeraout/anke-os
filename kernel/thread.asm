struc ts_thread
    .m_status: resw 1
    .m_taskOffset: resw 1
    .m_taskSegment: resw 1
    .m_memoryAllocationListOffset: resw 1
    .m_memoryAllocationListSegment: resw 1
endstruc

section .text

; struct ts_thread *thread_new(
;     struct ts_process *p_process,
;     uint16_t p_codeSegment,
;     uint16_t p_codeOffset,
;     uint16_t p_stackSize
; )
thread_new:
    %define p_processOffset (bp + 4)
    %define p_processSegment (bp + 6)
    %define p_codeOffset (bp + 8)
    %define p_codeSegment (bp + 10)
    %define p_stackSize (bp + 12)
    %define l_threadSegment (bp - 2)
    %define l_threadOffset (bp - 4)
    %define l_listElementSegment (bp - 6)
    %define l_listElementOffset (bp - 8)
    %define l_stackSegment (bp - 10)
    %define l_stackSegmentCount (bp - 12)

    push bp
    mov bp, sp
    sub sp, 12
    push es
    push di

    .allocateThread:
        ; DX:AX = malloc(sizeof(struct ts_thread));
        mov ax, ts_thread_size
        push ax
        call malloc
        add sp, 2

        ; If the allocation failed, return NULL.
        mov cx, ax
        or cx, dx
        jz .end

        ; Save the pointer to the thread structure
        mov [l_threadSegment], dx
        mov [l_threadOffset], ax

    .allocateListElement:
        ; DX:AX = malloc(sizeof(struct ts_listElement))
        mov ax, ts_listElement_size
        push ax
        call malloc
        add sp, 2

        ; If the allocation failed, go to the error handler.
        mov cx, ax
        or cx, dx
        jz .failedToAllocateListElement

        ; Save the pointer to the list element
        mov [l_listElementSegment], dx
        mov [l_listElementOffset], ax

    .allocateMemoryAllocation:
        ; DX:AX = malloc(sizeof(struct ts_memoryAllocation))
        mov ax, ts_memoryAllocation_size
        push ax
        call malloc
        add sp, 2

        ; If the allocation failed, go to the error handler.
        mov cx, ax
        or cx, dx
        jz .failedToAllocateMemoryAllocation

        ; Make ES:DI point to the memory allocation.
        mov es, dx
        mov di, ax

    .allocateStack:
        ; Determine the size of the stack in segments.
        mov ax, [p_stackSize]
        add ax, 15
        rcr ax, 1
        shr ax, 1
        shr ax, 1
        shr ax, 1

        mov [l_stackSegmentCount], ax

        ; Save the stack segment count
        push ax

        ; Allocate the stack
        push ax
        call mm_alloc
        add sp, 2

        ; If the allocation failed, go to the error handler.
        test ax, ax
        jz .failedToAllocateStack

        ; Save the stack segment
        mov [l_stackSegment], ax

    .initializeMemoryAllocation:
        ; Mark the stack in the memory allocation structure
        mov es:[di + ts_memoryAllocation.m_segment], ax
        pop ax
        mov es:[di + ts_memoryAllocation.m_segmentCount], ax

    .initializeListEntry:
        ; Initialize the list entry
        mov dx, es
        mov ax, di
        les di, [l_listElementOffset]
        mov es:[di + ts_listElement.m_dataSegment], dx
        mov es:[di + ts_listElement.m_dataOffset], ax
        xor ax, ax
        mov es:[di + ts_listElement.m_nextSegment], ax
        mov es:[di + ts_listElement.m_nextOffset], ax

    .initializeThread:
        mov dx, es
        mov ax, di
        les di, [l_threadOffset]
        mov word es:[di + ts_thread.m_status], E_THREADSTATUS_INITIALIZING
        mov es:[di + ts_thread.m_memoryAllocationListSegment], dx
        mov es:[di + ts_thread.m_memoryAllocationListOffset], ax

    .createTask:
        push es
        push di
        push word [p_processSegment]
        push word [p_processOffset]
        call task_new
        add sp, 8

        ; If the task creation failed, go to the error handler.
        mov cx, ax
        or cx, dx
        jz .failedToCreateTask

        ; Save the pointer to the task
        mov es:[di + ts_thread.m_taskSegment], dx
        mov es:[di + ts_thread.m_taskOffset], ax

    .initializeTaskContext:
        les di, es:[di + ts_thread.m_taskOffset]
        mov ax, [l_stackSegment]
        mov es:[di + ts_task.m_context + ts_taskContext.m_ss], ax
        mov ax, [p_stackSize]
        inc ax
        mov es:[di + ts_task.m_context + ts_taskContext.m_sp], ax
        mov word es:[di + ts_task.m_context + ts_taskContext.m_flags], 0x0200
        mov ax, [p_codeSegment]
        mov word es:[di + ts_task.m_context + ts_taskContext.m_cs], ax
        mov word es:[di + ts_task.m_context + ts_taskContext.m_ds], ax
        mov word es:[di + ts_task.m_context + ts_taskContext.m_es], ax
        mov ax, [p_codeOffset]
        mov word es:[di + ts_task.m_context + ts_taskContext.m_ip], ax

    .end:
        mov ax, [l_threadOffset]
        mov dx, [l_threadSegment]
        pop di
        pop es
        add sp, 12
        pop bp
        ret

    .failedToCreateTask:
        ; Free the stack
        les di, [l_listElementOffset]
        les di, es:[di + ts_listElement.m_dataOffset]

        xor ax, ax
        push ax
        push word es:[di + ts_memoryAllocation.m_segmentCount]
        push word es:[di + ts_memoryAllocation.m_segment]
        call mm_mark
        add sp, 6

    .failedToAllocateStack:
        ; Free the memory allocation
        push es
        push di
        call free
        add sp, 4

    .failedToAllocateMemoryAllocation:
        ; Free the list element
        push word [l_listElementSegment]
        push word [l_listElementOffset]
        call free
        add sp, 4

    .failedToAllocateListElement:
        ; Free the thread
        push word [l_threadSegment]
        push word [l_threadOffset]
        call free
        add sp, 4

        ; Return NULL
        xor ax, ax
        xor dx, dx
        jmp .end

    %undef p_processOffset
    %undef p_processSegment
    %undef p_codeOffset
    %undef p_codeSegment
    %undef p_stackSize
    %undef l_threadSegment
    %undef l_threadOffset
    %undef l_listElementSegment
    %undef l_listElementOffset

; void thread_start(struct ts_thread *p_thread)
thread_start:
    %define p_threadOffset (bp + 4)
    %define p_threadSegment (bp + 6)

    push bp
    mov bp, sp

    push es
    push di

    les di, [p_threadOffset]

    mov word es:[di + ts_thread.m_status], E_THREADSTATUS_READY

    pop di
    pop es

    pop bp
    ret

    %undef p_threadSegment
    %undef p_threadOffset

; void thread_destroy(struct ts_thread *p_thread)
thread_destroy:
    %define p_threadOffset (bp + 4)
    %define p_threadSegment (bp + 6)

    push bp
    mov bp, sp

    push es
    push di

    .destroyStack:
        les di, [p_threadOffset]
        les di, es:[di + ts_thread.m_memoryAllocationListOffset]
        les di, es:[di + ts_listElement.m_dataOffset]
        xor ax, ax
        push ax
        push word es:[di + ts_memoryAllocation.m_segmentCount]
        push word es:[di + ts_memoryAllocation.m_segment]
        call mm_mark
        add sp, 6

    .destroyMemoryAllocation:
        push es
        push di
        call free
        add sp, 4

    .destroyListElement:
        les di, [p_threadOffset]
        push word es:[di + ts_thread.m_memoryAllocationListSegment]
        push word es:[di + ts_thread.m_memoryAllocationListOffset]
        call free
        add sp, 4

    .destroyTask:
        push word es:[di + ts_thread.m_taskSegment]
        push word es:[di + ts_thread.m_taskOffset]
        call task_destroy
        add sp, 4

    .destroyThread:
        push word [p_threadSegment]
        push word [p_threadOffset]
        call free
        add sp, 4

    .end:
        pop di
        pop es

        pop bp
        ret

    %undef p_threadSegment
    %undef p_threadOffset
