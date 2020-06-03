bits 16
org 0x10000

section .text

_start:
    mov ax, 0x1000
    mov ds, ax
    mov es, ax
    mov ax, 0x9000
    mov ss, ax
    mov sp, 0xfffe
    jmp main16

%include "paging.inc"
%include "text16.inc"
%include "e820.inc"
%include "a20.inc"
%include "gdt.inc"
%include "text32.inc"

bits 16

section .text

main16:
    mov si, str_version
    mov bl, 0x70
    call text16_puts

    ; Read memory map
    call e820_readMemoryMap

    ; Enable A20 gate
    call a20_enable

    ; TODO: Load the kernel

    ; Load protected mode GDT
loadGDT:
    mov si, .str_loadGDT
    mov bl, 0x07
    call text16_puts

    lgdt [gdt_gdtr]

    mov si, .str_ok
    mov bl, 0x0a
    call text16_puts

    jmp enterProtectedMode

section .rodata

.str_loadGDT: db "Loading GDT... ", 0
.str_ok: db "OK", 13, 10, 0

section .text

    ; Enter protected mode
enterProtectedMode:
    mov si, .str_pmode
    mov bl, 0x07
    call text16_puts

    mov eax, cr0
    or ax, 1
    mov cr0, eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9fffc

    jmp dword 0x08:enterProtectedMode32

section .rodata
.str_pmode: db "Entering protected mode... ", 0

bits 32
section .text
enterProtectedMode32:
    call text32_init
    mov esi, .str_ok
    mov bl, 0x0a
    call text32_puts

    jmp halt32

section .rodata
.str_ok: db "OK", 13, 10, 0

section .text

    ; TODO: Map the lower memory (0x00000000-0x000fffff)
    ; TODO: Map the kernel to 0xc0000000
    ; Enable paging
    call paging_init

    ; TODO: Jump to the kernel

bits 16
halt16:
    cli
    hlt
    jmp halt16

bits 32
halt32:
    cli
    hlt
    jmp halt32

section .rodata
memoryMapSize: dw 0
str_version: db "AnkeOS bootloader 0.1.0", 13, 10, 13, 10, 0

