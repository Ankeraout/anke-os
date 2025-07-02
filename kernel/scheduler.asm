section .text

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

    .checkTaskListEmpty:
        mov ax, [g_scheduler_taskListSegment]
        or ax, [g_scheduler_taskListOffset]
        jz .taskListEmpty

    .checkCurrentTask:
        mov ax, [g_scheduler_currentTaskSegment]
        or ax, [g_scheduler_currentTaskOffset]
        jz .loopInitNoCurrentTask

    .loopInitCurrentTask:
        les di, [g_scheduler_currentTaskOffset]
        mov [l_stopConditionSegment], es
        mov [l_stopConditionOffset], di
        les di, es:[di + ts_listElement.m_nextOffset]

        mov ax, es
        or ax, di
        jz .loopInitCurrentTaskGetFirstTask
        mov [l_currentElementSegment], es
        mov [l_currentElementOffset], di

    .loopInit:
        xor ax, ax
        mov [l_newElementSegment], ax
        mov [l_newElementOffset], ax

    .loop:
        les di, [l_currentElementOffset]
        les di, es:[di + ts_listElement.m_dataOffset]
        les di, es:[di + ts_task.m_threadOffset]
        cmp word es:[di + ts_thread.m_status], E_THREADSTATUS_READY
        jnz .loopGetNextElement

    .loopFound:
        les di, [l_currentElementOffset]
        mov [l_newElementSegment], es
        mov [l_newElementOffset], di
        jmp .loopEnd

    .loopGetNextElement:
        les di, [l_currentElementOffset]
        les di, es:[di + ts_listElement.m_nextOffset]
        mov [l_currentElementSegment], es
        mov [l_currentElementOffset], di
        mov ax, es
        or ax, di
        jnz .loopCheckStopCondition

    .loopGetFirstElement:
        les di, [g_scheduler_taskListOffset]
        mov [l_currentElementSegment], es
        mov [l_currentElementOffset], di

    .loopCheckStopCondition:
        mov ax, [l_currentElementSegment]
        sub ax, [l_stopConditionSegment]
        mov dx, [l_currentElementOffset]
        sub dx, [l_stopConditionOffset]
        or ax, dx
        jnz .loop

    .loopEnd:
        mov ax, [l_newElementSegment]
        sub ax, [g_scheduler_currentTaskSegment]
        mov dx, [l_newElementOffset]
        sub dx, [g_scheduler_currentTaskOffset]
        or ax, dx
        jz .end

    .loadNewTask:
        mov ax, [g_scheduler_currentTaskSegment]
        or ax, [g_scheduler_currentTaskOffset]
        jz .loadNewTaskCheckNewElement

    .loadNewTaskUnloadCurrentTask:
        call task_save

    .loadNewTaskCheckNewElement:
        les di, [l_newElementOffset]
        mov [g_scheduler_currentTaskSegment], es
        mov [g_scheduler_currentTaskOffset], di
        mov ax, es
        or ax, di
        jz .end

    .loadNewTaskLoadNewTask:
        push word es:[di + ts_listElement.m_dataSegment]
        push word es:[di + ts_listElement.m_dataOffset]
        call task_load
        add sp, 4

    .end:
        pop di
        pop es
        add sp, 12
        pop bp
        ret

    .taskListEmpty:
        mov ax, [g_scheduler_currentTaskSegment]
        or ax, [g_scheduler_currentTaskOffset]
        jz .end
    
    .taskListEmptyUnloadCurrentTask:
        les di, [g_scheduler_currentTaskOffset]
        les di, es:[di + ts_listElement.m_dataOffset]
        les di, es:[di + ts_task.m_threadOffset]

        cmp word es:[di + ts_thread.m_status], E_THREADSTATUS_RUNNING
        jnz .taskListEmptyUnloadCurrentTaskFinalize

    .taskListEmptyUnloadCurrentTaskSetThreadReady:
        mov word es:[di + ts_thread.m_status], E_THREADSTATUS_READY

    .taskListEmptyUnloadCurrentTaskFinalize:
        call task_unload
        mov [g_scheduler_currentTaskSegment], ax
        mov [g_scheduler_currentTaskOffset], ax
        jmp .end

    .loopInitNoCurrentTask:
        les di, [g_scheduler_taskListOffset]
        mov [l_currentElementSegment], es
        mov [l_currentElementOffset], di
        mov [l_stopConditionSegment], es
        mov [l_stopConditionOffset], di
        jmp .loopInit

    .loopInitCurrentTaskGetFirstTask:
        les di, [g_scheduler_taskListOffset]
        mov [l_currentElementSegment], es
        mov [l_currentElementOffset], di
        jmp .loopInit

    %undef l_stopConditionSegment
    %undef l_stopConditionOffset
    %undef l_currentElementSegment
    %undef l_currentElementOffset
    %undef l_newElementSegment
    %undef l_newElementOffset

; void scheduler_run(void)
scheduler_run:
    mov ax, [g_scheduler_currentTaskSegment]
    or ax, [g_scheduler_currentTaskOffset]
    jnz task_resume

    .halt:
        hlt
        call scheduler_switch
        jmp scheduler_run

; int scheduler_add(struct ts_task *p_task)
scheduler_add:
    %define p_taskOffset (bp + 4)
    %define p_taskSegment (bp + 6)

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
    mov ax, [p_taskSegment]
    mov es:[di + ts_listElement.m_dataSegment], ax
    mov ax, [p_taskOffset]
    mov es:[di + ts_listElement.m_dataOffset], ax

    ; next = first
    mov ax, [g_scheduler_taskListSegment]
    mov es:[di + ts_listElement.m_nextSegment], ax
    mov ax, [g_scheduler_taskListOffset]
    mov es:[di + ts_listElement.m_nextOffset], ax
    
    ; first = new
    mov [g_scheduler_taskListSegment], es
    mov [g_scheduler_taskListOffset], di

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

    %undef p_taskSegment
    %undef p_taskOffset

; void scheduler_remove(struct ts_task *p_task)
scheduler_remove:
    %define p_taskOffset (bp + 4)
    %define p_taskSegment (bp + 6)

    push bp
    mov bp, sp

    ; If the task is the current running task, stop it.
    .checkCurrentTask:
        mov dx, [p_taskSegment]
        mov ax, [p_taskOffset]
        cmp dx, [g_scheduler_currentTaskSegment]
        jnz .removeFromTaskList
        cmp ax, [g_scheduler_currentTaskOffset]
        jz .stopCurrentTask

    .removeFromTaskList:
        push word [p_taskSegment]
        push word [p_taskOffset]
        push cs
        mov ax, g_scheduler_taskListOffset
        push ax
        call list_remove
        add sp, 8

    .end:
        pop bp
        ret

    .stopCurrentTask:
        call task_unload
        xor ax, ax
        mov [g_scheduler_currentTaskSegment], ax
        mov [g_scheduler_currentTaskOffset], ax
        jmp .removeFromTaskList

    %undef p_taskSegment
    %undef p_taskOffset

section .data
g_scheduler_taskListOffset: dw 0
g_scheduler_taskListSegment: dw 0
g_scheduler_currentTaskOffset: dw 0
g_scheduler_currentTaskSegment: dw 0
