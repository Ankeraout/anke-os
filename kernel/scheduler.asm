section .text

; void scheduler_run()
scheduler_run:
    %define l_loopStopTaskSegment (bp - 2)
    %define l_loopStopTaskOffset (bp - 4)

    mov bp, sp
    sub sp, 4

    ; We do not want to be interrupted during a task selection.
    call criticalSection_enter

    ; If there are no tasks to run, simply halt.
    mov ax, [g_scheduler_taskListSegment]
    or ax, [g_scheduler_taskListOffset]
    jz .halt

    ; Check if a task is currently running
    mov ax, [g_scheduler_currentTaskSegment]
    or ax, [g_scheduler_currentTaskOffset]

    ; If there is no running task, then the next task that needs to be
    ; checked is the first one.
    jz .initializeLoopFirstTask

    ; If there is already a task running, the first task that needs to be
    ; checked is the next one.
    call task_unload

    ; If the current task is RUNNING, make it READY.
    mov es, [g_scheduler_currentTaskSegment]
    mov di, [g_scheduler_currentTaskOffset]

    mov dx, es:[di + ts_listElement.m_dataSegment]
    mov di, es:[di + ts_listElement.m_dataOffset]
    mov es, dx

    mov dx, es:[di + ts_task.m_threadSegment]
    mov di, es:[di + ts_task.m_threadOffset]
    mov es, dx

    cmp word es:[di + ts_thread.m_status], E_THREADSTATUS_RUNNING
    jnz .initializeLoopNextTask

    mov word es:[di + ts_thread.m_status], E_THREADSTATUS_READY

    .initializeLoopNextTask:
        ; Load the current task
        mov es, [g_scheduler_currentTaskSegment]
        mov di, [g_scheduler_currentTaskOffset]

        ; If there is no next task, start looking from the first task.
        mov ax, es:[di + ts_listElement.m_nextSegment]
        or ax, es:[di + ts_listElement.m_nextOffset]
        jz .initializeLoopFirstTask

        ; If there is a next task, start looking from it.
        mov dx, es:[di + ts_listElement.m_nextSegment]
        mov di, es:[di + ts_listElement.m_nextOffset]
        mov es, dx
    
    .initializeLoop:
        ; Save the first checked task for stopping later
        mov [l_loopStopTaskSegment], es
        mov [l_loopStopTaskOffset], di

    .loop:
        ; Save pointer to the task list element
        push es
        push di

        ; Get the task object
        mov dx, es:[di + ts_listElement.m_dataSegment]
        mov di, es:[di + ts_listElement.m_dataOffset]
        mov es, dx

        ; Get the thread object
        mov dx, es:[di + ts_task.m_threadSegment]
        mov di, es:[di + ts_task.m_threadOffset]
        mov es, dx

        ; If the thread is ready, run it.
        cmp word es:[di + ts_thread.m_status], E_THREADSTATUS_READY
        jz .foundThread

        ; Otherwise, restore the task list element
        pop di
        pop es

        ; Check if the task list element is the last element to check
        ; (stop condition)
        mov ax, es
        xor ax, [l_loopStopTaskSegment]
        mov dx, di
        xor dx, [l_loopStopTaskOffset]
        or ax, dx
        jz .exitLoop

        ; There are more elements to check: get the next task list element.
        mov dx, es:[di + ts_listElement.m_nextSegment]
        mov di, es:[di + ts_listElement.m_nextOffset]
        mov es, dx
        jmp .loop

    .foundThread:
        ; Set the thread state to RUNNING
        mov word es:[di + ts_thread.m_status], E_THREADSTATUS_RUNNING

        ; Restore the task list element
        pop di
        pop es

        ; Make ES:DI point to the task object
        ; Load the task
        push word es:[di + ts_listElement.m_dataSegment]
        push word es:[di + ts_listElement.m_dataOffset]
        call task_load

        ; Save the new running task
        mov [g_scheduler_currentTaskSegment], es
        mov [g_scheduler_currentTaskOffset], di

        ; We are no longer in a critical section.
        call criticalSection_leave

        ; Resume the task
        call task_resume

    .exitLoop:
    .halt:
        ; No task can be run: just halt and loop again.
        ; When an interrupt occurs, the scheduler will be unblocked.
        xor ax, ax
        mov [g_scheduler_currentTaskSegment], ax
        mov [g_scheduler_currentTaskOffset], ax
        call task_clear
        call criticalSection_leave
        hlt
        jmp scheduler_run

    .initializeLoopFirstTask:
        ; Load the first task
        mov es, [g_scheduler_taskListSegment]
        mov di, [g_scheduler_taskListOffset]
        jmp .initializeLoop

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

section .data
g_scheduler_taskListOffset: dw 0
g_scheduler_taskListSegment: dw 0
g_scheduler_currentTaskOffset: dw 0
g_scheduler_currentTaskSegment: dw 0
