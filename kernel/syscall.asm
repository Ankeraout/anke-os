section .text

; int syscall_init()
; Summary: This function initializes the interrupt vector for int 0x80.
; Returns: Always 0.
syscall_init:
    push es

    xor ax, ax
    mov es, ax

    mov word es:[0x80 * 4], isr_int80
    mov es:[0x80 * 4 + 2], cs

    pop es
    ret

; void isr_int80()
; This function is called when an int 0x80 opcode is executed.
isr_int80:
    add ax, cx
    iret

section .rodata
g_syscall_jump_table:

