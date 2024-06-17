bits 16
cpu 8086
section .bootstrap16

extern main

global _start
_start:
    ; Initialize segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ax, 0x9000
    mov sp, 0xf000
    jmp 0x0000:.reload_cs

.halt:
    ; Halt the processor
    hlt
    jmp .halt

.unsupported_cpu:
    mov ax, g_str_unsupported_cpu
    push ax
    call puts
    add sp, 2
    jmp .halt

.reload_cs:
    ; Save the BIOS boot drive number
    mov [g_bootDrive], dl

    ; Print announce
    mov ax, g_str_announce
    push ax
    call puts
    add sp, 2
    
    mov ax, g_str_hw_detect
    push ax
    call puts
    add sp, 2

    ; Detect hardware
    call hw_detect_cpu

    cmp al, 6
    jb .unsupported_cpu

.reload_gdt:
    cpu 386
    mov ax, g_str_disable_ints
    push ax
    call puts
    add sp, 2

    ; Disable interrupts
    cli

    ; Disable NMI
    in al, 0x70
    or al, 0x80
    out 0x70, al
    
    mov ax, g_str_init_gdt
    push ax
    call puts
    add sp, 2

    lgdt [g_gdtr]
    
    mov ax, g_str_init_pmode
    push ax
    call puts
    add sp, 2

    mov eax, cr0
    or al, 1
    mov cr0, eax

    jmp dword 0x0008:.next

.next:
bits 32
    ; Reload segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9f000

    call main
    jmp .halt

bits 16
%include "bootstrap16/stdio.inc"
%include "bootstrap16/strings.inc"
%include "bootstrap16/hw_cpu.inc"
%include "bootstrap16/gdt.inc"

g_bootDrive db 0
