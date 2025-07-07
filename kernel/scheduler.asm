section .text

; int scheduler_init()
scheduler_init:
    push cs
    mov ax, scheduler_tick
    push ax
    xor ax, ax
    push ax
    call irq_addHandler
    add sp, 6
    ret

; void scheduler_switch()
scheduler_switch:
    %define l_stopConditionSegment (bp - 2)
    %define l_stopConditionOffset (bp - 4)
    %define l_currentElementSegment (bp - 6)
    %define l_currentElementOffset (bp - 8)
    %define l_newElementSegment (bp - 10)
    %define l_newElementOffset (bp - 12)

    push bp
    mov bp, sp
    sub sp, 12
    push es
    push di

    call criticalSection_enter

    .checkThreadListEmpty:
        mov ax, [g_scheduler_threadListOffset]
        or ax, [g_scheduler_threadListSegment]
        jz .end

    .determineStopCondition:
        mov ax, [g_scheduler_currentThreadOffset]
        or ax, [g_scheduler_currentThreadSegment]
        jnz .determineStopCondition.currentThreadNotNull

    .determineStopCondition.currentThreadNull:
        les di, [g_scheduler_threadListOffset]
        mov [l_stopConditionOffset], di
        mov [l_stopConditionSegment], es
        jmp .determineCurrentElement
    
    .determineStopCondition.currentThreadNotNull:
        les di, [g_scheduler_currentThreadOffset]
        les di, es:[di + ts_listElement.m_dataOffset]
        cmp word es:[di + ts_thread.m_status], E_THREADSTATUS_RUNNING
        jnz .determineStopCondition.setStopCondition

    .determineStopCondition.currentThreadRunning:
        mov word es:[di + ts_thread.m_status], E_THREADSTATUS_READY

    .determineStopCondition.setStopCondition:
        les di, [g_scheduler_currentThreadOffset]
        mov [l_stopConditionOffset], di
        mov [l_stopConditionSegment], es

    .determineCurrentElement:
        ; les di, [l_stopConditionOffset]
        mov ax, es:[di + ts_listElement.m_nextOffset]
        or ax, es:[di + ts_listElement.m_nextSegment]
        jnz .determineCurrentElement.nextNotNull

    .determineCurrentElement.nextNull:
        les di, [g_scheduler_threadListOffset]
        jmp .initializeNewElement

    .determineCurrentElement.nextNotNull:
        les di, es:[di + ts_listElement.m_nextOffset]

    .initializeNewElement:
        mov [l_currentElementOffset], di
        mov [l_currentElementSegment], es
        xor ax, ax
        mov word [l_newElementOffset], ax
        mov word [l_newElementSegment], ax

    .loop:
    .loop.loadCurrentElement:
        les di, [l_currentElementOffset]
        les di, es:[di + ts_listElement.m_dataOffset]

    .loop.checkThreadStatus:
        cmp word es:[di + ts_thread.m_status], E_THREADSTATUS_READY
        les di, [l_currentElementOffset]
        jz .loop.threadReady

    .loop.threadNotReady:
        mov ax, es:[di + ts_listElement.m_nextOffset]
        or ax, es:[di + ts_listElement.m_nextSegment]
        jz .loop.threadNotReady.nextNotNull

    .loop.threadNotReady.nextNull:
        les di, [g_scheduler_threadListOffset]
        mov [l_currentElementOffset], di
        mov [l_currentElementSegment], es
        jmp .loop.end

    .loop.threadReady:
        mov [l_newElementOffset], di
        mov [l_newElementSegment], es
        jmp .loop.checkCondition

    .loop.threadNotReady.nextNotNull:
        les di, es:[di + ts_listElement.m_nextOffset]
        mov [l_currentElementOffset], di
        mov [l_currentElementSegment], es

    .loop.checkCondition:
        mov ax, [l_newElementOffset]
        or ax, [l_newElementSegment]
        jnz .loop.end

        mov ax, [l_currentElementOffset]
        sub ax, [l_stopConditionOffset]
        mov dx, [l_currentElementSegment]
        sub dx, [l_stopConditionSegment]
        or ax, dx
        jnz .loop

    .loop.end:
        mov ax, [l_newElementOffset]
        sub ax, [g_scheduler_currentThreadOffset]
        mov dx, [l_newElementSegment]
        sub dx, [g_scheduler_currentThreadSegment]
        or ax, dx
        jz .checkNewElementNull

    .savePreviousTask:
        mov ax, [g_scheduler_currentThreadOffset]
        or ax, [g_scheduler_currentThreadSegment]
        jz .checkNewElementNull
        
        les di, [g_scheduler_currentThreadOffset]
        les di, es:[di + ts_listElement.m_dataOffset]
        push es
        lea ax, [di + ts_thread.m_context]
        push ax
        call task_save
        add sp, 4

    .checkNewElementNull:
        mov ax, [l_newElementOffset]
        or ax, [l_newElementSegment]
        jz .setCurrentThread

    .loadNewTask:
        les di, [l_currentElementOffset]
        les di, es:[di + ts_listElement.m_dataOffset]
        push es
        lea ax, [di + ts_thread.m_context]
        push ax
        call task_load
        add sp, 4
        mov word es:[di + ts_thread.m_status], E_THREADSTATUS_RUNNING

    .setCurrentThread:
        les di, [l_newElementOffset]
        mov [g_scheduler_currentThreadOffset], di
        mov [g_scheduler_currentThreadSegment], es

    .end:
        call criticalSection_leave
        pop di
        pop es
        add sp, 12
        pop bp
        ret

    %undef l_stopConditionSegment
    %undef l_stopConditionOffset
    %undef l_currentElementSegment
    %undef l_currentElementOffset
    %undef l_newElementSegment
    %undef l_newElementOffset

; void scheduler_run(void)
scheduler_run:
    mov ax, [g_scheduler_currentThreadSegment]
    or ax, [g_scheduler_currentThreadOffset]
    jnz task_resume

    .halt:
        hlt
        call scheduler_switch
        jmp scheduler_run

; int scheduler_add(struct ts_thread *p_thread)
scheduler_add:
    %define p_threadOffset (bp + 4)
    %define p_threadSegment (bp + 6)

    push bp
    mov bp, sp

    ; Allocate memory for the list element
    mov ax, ts_listElement_size
    push ax
    call malloc
    add sp, 2

    test dx, dx
    jz .allocationError

    call criticalSection_enter

    push es
    push di

    mov es, dx
    mov di, ax

    ; Initialize list element
    mov ax, [p_threadSegment]
    mov es:[di + ts_listElement.m_dataSegment], ax
    mov ax, [p_threadOffset]
    mov es:[di + ts_listElement.m_dataOffset], ax

    ; next = first
    mov ax, [g_scheduler_threadListSegment]
    mov es:[di + ts_listElement.m_nextSegment], ax
    mov ax, [g_scheduler_threadListOffset]
    mov es:[di + ts_listElement.m_nextOffset], ax
    
    ; first = new
    mov [g_scheduler_threadListSegment], es
    mov [g_scheduler_threadListOffset], di

    call criticalSection_leave

    pop di
    pop es

    xor ax, ax

.end:
    pop bp
    ret

.allocationError:
    mov ax, 1
    jmp .end

    %undef p_threadSegment
    %undef p_threadOffset

; void scheduler_remove(struct ts_thread *p_thread)
scheduler_remove:
    %define p_threadOffset (bp + 4)
    %define p_threadSegment (bp + 6)

    push bp
    mov bp, sp

    ; If the task is the current running task, stop it.
    .checkCurrentTask:
        mov dx, [p_threadSegment]
        mov ax, [p_threadOffset]
        cmp dx, [g_scheduler_currentThreadSegment]
        jnz .removeFromTaskList
        cmp ax, [g_scheduler_currentThreadOffset]
        jz .stopCurrentTask

    .removeFromTaskList:
        push word [p_threadSegment]
        push word [p_threadOffset]
        push cs
        mov ax, g_scheduler_threadListOffset
        push ax
        call list_remove
        add sp, 8

    .end:
        pop bp
        ret

    .stopCurrentTask:
        les di, [g_scheduler_currentThreadOffset]
        les di, es:[di + ts_listElement.m_dataOffset]
        push di
        lea ax, [di + ts_thread.m_context]
        push ax
        call task_save
        add sp, 4

        xor ax, ax
        mov [g_scheduler_currentThreadSegment], ax
        mov [g_scheduler_currentThreadOffset], ax
        jmp .removeFromTaskList

    %undef p_threadSegment
    %undef p_threadOffset

; void scheduler_tick(void)
scheduler_tick:
    call scheduler_switch
    iret

section .data
align 2
g_scheduler_threadListOffset: dw 0
g_scheduler_threadListSegment: dw 0
g_scheduler_currentThreadOffset: dw 0
g_scheduler_currentThreadSegment: dw 0
