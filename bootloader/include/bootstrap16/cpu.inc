align 2

cpu_check:
    push bp
    mov bp, sp
    sub sp, 4

    ; Save #UD handler and replace it with ours
    push ds

    xor ax, ax
    mov ds, ax
    
    mov ax, [0x0018]
    mov [ss:bp - 4], ax
    mov ax, [0x001a]
    mov [ss:bp - 2], ax

    mov ax, cs
    mov [0x001a], ax
    mov ax, .unsupported_cpu
    mov [0x0018], ax

    pop ds

    ; Check if the CPU supports x86_64
    cpu 586

    mov eax, 0x80000001
    cpuid
    test edx, (1 << 29)
    
    cpu 8086
    
    jz .unsupported_cpu

    ; Restore #UD handler
    push ds

    xor ax, ax
    mov ds, ax

    mov ax, [ss:bp - 4]
    mov [0x0018], ax
    mov ax, [ss:bp - 2]
    mov [0x001a], ax

    pop ds

    ; Function epilogue
    add sp, 4
    pop bp
    ret

.unsupported_cpu:
    mov ax, str_unsupported_cpu
    push ax
    call puts
    add sp, 2
    jmp halt
