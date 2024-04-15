section .bootstrap32
bits 32

extern main64

%include "bootstrap32/gdt.inc"
%include "bootstrap32/paging.inc"
%include "bootstrap32/stdio.inc"

global main32
main32:
    call stdio_init

    push str_announce
    call puts
    add esp, 4

    call gdt_init

    ; Initialize paging structures
    call paging_init

    ; Enable PAE
    mov eax, cr4
    or al, 1 << 5
    mov cr4, eax

    ; Load CR3
    mov eax, 0x1000
    mov cr3, eax

    ; Enable long mode
    mov ecx, 0xc0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ; Reload segments and jump to 64-bit code
    mov ax, 0x0020
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp dword 0x0018:main64

str_announce db "AnkeOS bootloader 0.1.0 (protected mode bootstrap)", 10, 0
