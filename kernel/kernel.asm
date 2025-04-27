bits 16
cpu 8086

_start:
    jmp main

%include "kernel/mm.asm"
%include "kernel/printk.asm"
%include "kernel/stdlib.asm"
%include "kernel/string.asm"

main:
    mov ax, 0x1000
    mov ds, ax
    mov es, ax
    mov ax, 0x7000
    mov ss, ax
    xor sp, sp

    mov ax, g_kernel_msg_boot
    push ax
    push ds
    call printk
    add sp, 4

    ; Initialize the memory manager
    call mm_init

    cli
    hlt

g_kernel_msg_boot:
    db "AnkeOS kernel x86_16 0.1.0 (", __DATE__, " ", __TIME__, ")", 13, 10, 0

g_kernel_end:
