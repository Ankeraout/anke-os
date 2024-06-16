bits 16
cpu 8086

main:
    ; Initialize segment registers
    mov ax, 0x1000
    mov ds, ax
    mov es, ax
    mov ax, 0x9000
    mov sp, 0xf000

    ; Save the BIOS boot drive number
    mov [g_bootDrive], dl

    ; Print announce
    mov ax, g_str_announce
    push ax
    call puts
    add sp, 2

    ; Detect hardware
    call hw_detect

    ; Halt the processor
    .halt:
        hlt
        jmp .halt

%include "stdio.inc"
%include "strings.inc"
%include "hw.inc"
%include "hw_cpu.inc"
%include "hw_pci.inc"

g_bootDrive db 0
