section .text

test:
    %define l_processSegment (bp - 2)
    %define l_processOffset (bp - 4)
    %define l_thread1Segment (bp - 6)
    %define l_thread1Offset (bp - 8)
    %define l_thread2Segment (bp - 10)
    %define l_thread2Offset (bp - 12)

    push bp
    mov bp, sp

    sub sp, 12

.createProcess:
    xor ax, ax
    push ax
    push ax
    call process_new
    add sp, 4

    cmp dx, 0
    jz .failedToCreateProcess

    mov [l_processSegment], dx
    mov [l_processOffset], ax

.createThread1:
    mov ax, 4096
    push ax
    mov ax, thread_1_main
    push cs
    push ax
    push word [l_processSegment]
    push word [l_processOffset]
    call thread_new
    add sp, 10
    
    cmp dx, 0
    jz .failedToCreateThread1

    mov [l_thread1Segment], dx
    mov [l_thread1Offset], ax

    push dx
    push ax
    call thread_start
    add sp, 4

.createThread2:
    mov ax, 4096
    push ax
    mov ax, thread_2_main
    push cs
    push ax
    push word [l_processSegment]
    push word [l_processOffset]
    call thread_new
    add sp, 10
    
    cmp dx, 0
    jz .failedToCreateThread2

    mov [l_thread2Segment], dx
    mov [l_thread2Offset], ax

    push dx
    push ax
    call thread_start
    add sp, 4

.installIsr0:
    call criticalSection_enter

    xor ax, ax
    mov es, ax
    mov word es:[0x0020], isr_irq0
    mov word es:[0x0022], C_KERNEL_SEGMENT

    call criticalSection_leave

.startRunning:
    call scheduler_run

.end:
    add sp, 8
    pop bp
    ret

.failedToCreateThread2:
    mov ax, 3
    jmp .end

.failedToCreateThread1:
    mov ax, 2
    jmp .end

.failedToCreateProcess:
    mov ax, 1
    jmp .end

    %undef l_thread1Segment
    %undef l_thread1Offset
    %undef l_thread2Segment
    %undef l_thread2Offset
    %undef l_processSegment
    %undef l_processOffset

thread_1_main:
    mov ax, 0xb800
    mov es, ax
    xor di, di
    mov byte es:[di], '0'
    mov byte es:[di + 1], 0x1f
    
    .loop:
        inc byte es:[di]
        cmp byte es:[di], '9' + 1
        jz .reset
        hlt
        jmp .loop
    
    .reset:
        mov byte es:[di], '0' - 1
        jmp .loop

thread_2_main:
    mov ax, 0xb800
    mov es, ax
    mov di, 2
    mov byte es:[di], '9'
    mov byte es:[di + 1], 0x1f
    
    .loop:
        dec byte es:[di]
        cmp byte es:[di], '0' - 1
        jz .reset
        hlt
        jmp .loop
    
    .reset:
        mov byte es:[di], '9' + 1
        jmp .loop

isr_irq0:
    M_TASK_SAVE_CONTEXT

    mov al, 0x20
    out 0x20, al

    jmp scheduler_run
