bits 16
org 0x10000

_start:
    mov ax, 0x1000
    mov ds, ax
    mov es, ax
    mov ax, 0x9000
    mov ss, ax
    mov sp, 0xfffe
    jmp main

%include "text16.inc"
%include "e820.inc"
%include "a20.inc"
%include "gdt.inc"

bits 16

main:
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

    jmp halt16

    mov si, .str_ok
    mov bl, 0x07
    call text16_puts

.str_loadGDT: db "Loading GDT... ", 0
.str_ok: db "OK", 13, 10, 0

    ; Enter protected mode
enterProtectedMode:
    mov si, .str_pmode
    mov bl, 0x07
    call text16_puts

    mov eax, cr0
    or ax, 1
    mov cr0, eax

bits 32

    jmp .next
.next:
    jmp halt32

.str_pmode: db "Entering protected mode... ", 0
.str_ok: db "OK", 13, 10, 0


    ; TODO: Map the lower memory (0x00000000-0x000fffff)
    ; TODO: Map the kernel to 0xc0000000
    ; TODO: Enable paging
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

memoryMapSize: dw 0
str_version: db "AnkeOS bootloader 0.1.0", 13, 10, 13, 10, 0

