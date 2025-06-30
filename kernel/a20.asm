section .text

; void a20_relocateKernel(void)
a20_relocateKernel:
    ; Enable the A20 line. If it is not enabled, do nothing.
    call a20_enable
    test ax, ax
    jz .end

    ; Relocate the kernel
    mov ax, g_kernel_end - C_KERNEL_OFFSET
    push ax
    push cs
    mov ax, C_KERNEL_OFFSET
    push ax
    mov ax, 0xffff
    push ax
    mov ax, C_KERNEL_OFFSET
    push ax
    call memcpy
    add sp, 10

    ; Jump to higher memory kernel
    jmp 0xffff:.end

    .end:
        mov ax, cs
        mov ds, ax
        mov es, ax
        mov ss, ax
        xor ax, ax
        ret

; int a20_isEnabled(void)
a20_isEnabled:
    call criticalSection_enter

    push ds
    push es

    xor ax, ax
    mov ds, ax
    not ax
    mov es, ax

    push word [0x0000]
    mov byte [0x0000], 0xaa
    mov byte es:[0x0010], 0x55
    cmp byte [0x0000], 0xaa
    pop word [0x0000]
    jz .enabled

    .disabled:
        xor ax, ax
        jmp .end

    .enabled:
        mov ax, 1

    .end:
        call criticalSection_leave
        pop es
        pop ds
        ret

; int a20_enable(void)
a20_enable:
    ; Check if the A20 line is already enabled.
    call a20_isEnabled
    test ax, ax
    jnz .enabled

    .biosMethod:
        ; Enable A20 gate using the BIOS
        mov ax, 0x2401
        int 0x15
        
        call a20_isEnabled
        test ax, ax
        jnz .enabled

    ; TODO: implement keyboard controller method
    ; TODO: implement fast A20 method

    .notEnabled:
        xor ax, ax
        jmp .end

    .enabled:
        mov ax, 1

    .end:
        ret
