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
%include "bootstrap16/mmap.inc"
%include "bootstrap16/sysinfo.inc"
%include "bootstrap16/vbe.inc"

main16:
    mov ax, str_announce
    push ax
    call puts
    add sp, 2

    call cpu_check

cpu x64
    call a20_init

    test ax, ax
    jnz .a20_error

    call vbe_init
    call mmap_init
    call sysinfo_init

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

    mov ebx, 0x1000

    jmp dword 0x0008:main32

.a20_error:
    push word str_a20_nok
    call puts
    add sp, 2

halt:
    cli
    hlt
    jmp halt
