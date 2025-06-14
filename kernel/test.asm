section .text

test:
    %define l_threadSegment (bp - 8)
    %define l_threadOffset (bp - 6)
    %define l_processSegment (bp - 4)
    %define l_processOffset (bp - 2)

    push bp
    mov bp, sp

    sub sp, 8

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

.createThread:
    mov ax, 4096
    push ax
    mov ax, thread_1_main
    push ax
    push cs
    push word [l_processOffset]
    push word [l_processSegment]
    call thread_new
    add sp, 10
    
    cmp dx, 0
    jz .failedToCreateThread

    mov [l_threadSegment], dx
    mov [l_threadOffset], ax

    push es
    push di

    mov es, dx
    mov di, ax

    push word es:[di + ts_thread.m_taskOffset]
    push word es:[di + ts_thread.m_taskSegment]
    
    call task_load

    pop di
    pop es

    call task_resume

.end:
    add sp, 8
    pop bp
    ret

.failedToCreateThread:
    mov ax, 2
    jmp .end

.failedToCreateProcess:
    mov ax, 1
    jmp .end

    %undef l_threadSegment
    %undef l_threadOffset
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
