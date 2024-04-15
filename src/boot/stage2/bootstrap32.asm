section .bootstrap32
bits 32

extern main64

%include "boot/stage2/bootstrap32/gdt.inc"
%include "boot/stage2/bootstrap32/paging.inc"
%include "boot/stage2/bootstrap32/stdio.inc"

global main32
main32:
    call stdio_init

    push str_announce
    call puts
    add esp, 4

    call gdt_init

    push str_gdt
    call puts
    add esp, 4

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

str_announce    db "32-bit bootstrap procedure started.", 13, 10, 0
str_gdt         db "GDT loaded.", 13, 10, 0
