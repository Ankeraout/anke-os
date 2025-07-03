%define C_IRQ_COUNT 16

section .text

; int irq_init(void)
irq_init:
    %define l_irqCount (bp - 2)
    %define l_listEntrySegment (bp - 4)
    %define l_listEntryOffset (bp - 6)

    push bp
    mov bp, sp
    sub sp, 6
    push es
    push di
    push bx

    ; Register BIOS IRQ handlers
    mov word [l_irqCount], 0
    mov bx, g_irq_vectorTable

    call criticalSection_enter

    .registerBiosIrqLoop:
        mov ax, ts_listElement_size
        push ax
        call malloc
        add sp, 2

        mov cx, ax
        or cx, dx
        jz .failedToAllocateMemory

        mov [l_listEntrySegment], dx
        mov [l_listEntryOffset], ax

        ; Get interrupt vector
        mov ax, [l_irqCount]
        xlat
        shl ax, 1
        shl ax, 1
        mov di, ax

        xor ax, ax
        mov es, ax

        mov ax, es:[di]
        mov dx, es:[di + 2]

        push es
        push di

        ; Initialize list element
        les di, [l_listEntryOffset]
        mov es:[di + ts_listElement.m_dataSegment], dx
        mov es:[di + ts_listElement.m_dataOffset], ax
        xor ax, ax
        mov word es:[di + ts_listElement.m_nextSegment], ax
        mov word es:[di + ts_listElement.m_nextOffset], ax

        ; Save list element pointer
        mov al, 4
        mul byte [l_irqCount]
        xchg ax, di
        mov [di + g_irq_isrListTable], ax
        mov [di + g_irq_isrListTable + 2], es

        ; Change the handler in the IVT
        mov di, [l_irqCount]
        shl di, 1
        mov ax, [di + g_irq_isrTable]
        pop di
        pop es
        mov es:[di], ax
        mov es:[di + 2], cs
        
        ; Increment index
        inc word [l_irqCount]
        cmp byte [l_irqCount], C_IRQ_COUNT
        jnz .registerBiosIrqLoop

        xor ax, ax

    .end:
        call criticalSection_leave
        pop bx
        pop di
        pop es
        add sp, 6
        pop bp
        ret

    .failedToAllocateMemory:
        ; TODO: free memory
        mov ax, 1
        jmp .end

    %undef l_irqCount
    %undef l_listEntrySegment
    %undef l_listEntryOffset

irq_service:
    ; Save context
    mov [g_task_currentTaskContext + ts_taskContext.m_ax], ax
    mov [g_task_currentTaskContext + ts_taskContext.m_bx], bx
    mov [g_task_currentTaskContext + ts_taskContext.m_cx], cx
    mov [g_task_currentTaskContext + ts_taskContext.m_dx], dx
    mov [g_task_currentTaskContext + ts_taskContext.m_bp], bp
    mov [g_task_currentTaskContext + ts_taskContext.m_si], si
    mov [g_task_currentTaskContext + ts_taskContext.m_ds], ds
    mov [g_task_currentTaskContext + ts_taskContext.m_di], di
    mov [g_task_currentTaskContext + ts_taskContext.m_es], es
    mov [g_task_currentTaskContext + ts_taskContext.m_ss], ss
    pop word [g_task_currentTaskContext + ts_taskContext.m_ip]
    pop word [g_task_currentTaskContext + ts_taskContext.m_cs]
    pop word [g_task_currentTaskContext + ts_taskContext.m_flags]
    mov [g_task_currentTaskContext + ts_taskContext.m_sp], sp

    ; Call all ISRs
    push bp
    mov bp, sp

    push ax
    push es
    push di

    mov al, 4
    mul byte [g_isr_irq]
    mov di, ax

    les di, [di + g_irq_isrListTable]

    .loop:
        mov ax, es
        or ax, di
        jz .end

        pushf
        call far [es:di + ts_listElement.m_dataOffset]
    
    .next:
        les di, [es:di + ts_listElement.m_nextOffset]
        jmp .loop

    .end:
        call task_resume

; int irq_addHandler(int p_irq, void *p_handler)
irq_addHandler:
    %define p_irq (bp + 4)
    %define p_handlerOffset (bp + 6)
    %define p_handlerSegment (bp + 8)

    push bp
    mov bp, sp

    push es
    push di
    push si

    mov ax, ts_listElement_size
    push ax
    call malloc
    add sp, 2

    mov cx, ax
    or cx, dx
    jz .error

    mov es, dx
    mov di, ax

    ; Set handler address
    mov ax, [p_handlerSegment]
    mov es:[di + ts_listElement.m_dataSegment], ax
    mov ax, [p_handlerOffset]
    mov es:[di + ts_listElement.m_dataOffset], ax
    
    ; Set next
    mov si, [p_irq]
    shl si, 1
    shl si, 1
    add si, g_irq_isrListTable
    mov ax, [si]
    mov es:[di + ts_listElement.m_nextOffset], ax
    mov ax, [si + 2]
    mov es:[di + ts_listElement.m_nextSegment], ax
    
    ; Replace current list element
    mov [si], di
    mov [si + 2], es

    ; Return 0
    xor ax, ax

    .end:
        pop si
        pop di
        pop es
        pop bp
        ret

    .error:
        mov ax, 1
        jmp .end

    %undef p_irq
    %undef p_handlerSegment
    %undef p_handlerOffset

; void irq_removeHandler(int p_irq, void *p_handler)
irq_removeHandler:
    %define p_irq (bp - 2)
    %define p_handlerSegment (bp - 4)
    %define p_handlerOffset (bp - 6)

    push bp
    mov bp, sp

    mov ax, [p_irq]
    shl ax, 1
    shl ax, 1
    add ax, g_irq_isrListTable
    
    push word [p_handlerSegment]
    push word [p_handlerOffset]
    push ds
    push ax
    call list_remove
    add sp, 8

    pop bp
    ret

    %undef p_irq
    %undef p_handlerSegment
    %undef p_handlerOffset

section .rodata
g_irq_vectorTable:
    db 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    db 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77

g_irq_isrTable:
    dw isr_irq0, isr_irq1, isr_irq2, isr_irq3 
    dw isr_irq4, isr_irq5, isr_irq6, isr_irq7
    dw isr_irq8, isr_irq9, isr_irq10, isr_irq11
    dw isr_irq12, isr_irq13, isr_irq14, isr_irq15

section .data
align 2
g_irq_isrListTable:
    times C_IRQ_COUNT dd 0
