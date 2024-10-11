bits 16
cpu 8086

section .bootstrap16

; TODO:
; - Detect CPU
; - Obtain memory map from BIOS
; - Detect PCI bus using BIOS
; - Protected mode
; - PCI driver
; - PATA driver
; - FAT driver (12/16 at least)

global _start
_start:
    mov ax, 0x1000
    mov ds, ax
    mov es, ax
    mov ax, 0x8f00
    mov ss, ax
    xor sp, sp

halt:
    cli
    hlt
    jmp halt
