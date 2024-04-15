section .bootstrap16

bits 16
cpu 8086

extern main32
global _start
_start:
    mov ax, 0x1000
    mov ds, ax
    mov es, ax
    mov ax, 0x8f00
    mov ss, ax
    xor sp, sp
    jmp 0x1000:main16

%include "bootstrap16/cpu.inc"
%include "bootstrap16/stdio.inc"
%include "bootstrap16/strings.inc"
%include "bootstrap16/a20.inc"
%include "bootstrap16/nmi.inc"
%include "bootstrap16/gdt.inc"

main16:
    mov ax, str_announce
    push ax
    call puts
    add sp, 2

    call cpu_check
    call a20_init

    test ax, ax
    jnz .a20_error

    ; Disable interrupts (including NMI)
    cli
    call nmi_disable

    ; Load GDT and start executing protected mode code
    cpu 386
    lgdt [gdtr]

    ; Enable protected mode
    mov eax, cr0
    or al, 1
    mov cr0, eax
    
    ; Reload segment registers and jump to main32 in 32-bit mode
    mov ax, 0x0010
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9f000
    jmp dword 0x0008:main32

.a20_error:
    mov ax, str_a20_nok
    push ax
    call puts
    add sp, 2

halt:
    cli
    hlt
    jmp halt
