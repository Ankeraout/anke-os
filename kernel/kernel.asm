bits 16
cpu 8086

%define C_KERNEL_SEGMENT 0x1000

section .text
_start:
    jmp main

%include "kernel/thread_constants.asm"

%include "kernel/critical_section.asm"
%include "kernel/list.asm"
%include "kernel/memory_allocation.asm"
%include "kernel/mm.asm"
%include "kernel/printk.asm"
%include "kernel/process.asm"
%include "kernel/scheduler.asm"
%include "kernel/sequence.asm"
%include "kernel/stdlib.asm"
%include "kernel/string.asm"
%include "kernel/task.asm"
%include "kernel/test.asm"
%include "kernel/thread.asm"

section .text

main:
    mov ax, C_KERNEL_SEGMENT
    mov ds, ax
    mov es, ax
    mov ax, 0x7000
    mov ss, ax
    xor sp, sp

    mov ax, g_kernel_msg_boot
    push ds
    push ax
    call printk
    add sp, 4

    mov ax, g_kernel_sequence_init
    push ax
    call sequence_run
    add sp, 2

    cli
    hlt

section .rodata
g_kernel_msg_boot:
    db "AnkeOS kernel x86_16 0.1.0 (", __DATE__, " ", __TIME__, ")", 13, 10, 0

g_kernel_sequence_init_name: db "Kernel initialization sequence", 0
g_kernel_sequence_init_mm_name: db "Memory manager initialization", 0
g_kernel_sequence_init_test_name: db "Kernel test", 0

g_kernel_sequence_init:
istruc ts_sequence
    at .m_name, dw g_kernel_sequence_init_name
    at .m_count, dw 2
    at .m_elements
iend
istruc ts_sequenceElement
    at .m_name, dw g_kernel_sequence_init_mm_name
    at .m_func, dw mm_init
iend
istruc ts_sequenceElement
    at .m_name, dw g_kernel_sequence_init_test_name
    at .m_func, dw test
iend
