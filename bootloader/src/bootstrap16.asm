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

main16:
    ; Check the CPU
    call cpu_check
    test ax, ax
    jnz .error_cpu

    ; After this point, we know that the CPU is supported, so we can use 32-bit
    ; instructions.
    cpu x64

    ; Enable the A20 gate
    call a20_enable
    test ax, ax
    jnz .error_a20

    ; Initialize VBE (note that there is no error checking, because vbe_init
    ; failing is not considered a fatal error).
    call vbe_init

    ; Initialize the memory map.
    call mmap_init
    test ax, ax
    jnz .error_mmap

    ; Initialize the system information structure.
    call sysinfo_init
    test ax, ax
    jnz .error_sysinfo

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

cpu 8086
    .error_cpu:
        mov ax, str_error_cpu
        jmp .fatal_error
        
    .error_a20:
        mov ax, str_error_a20
        jmp .fatal_error

    .error_mmap:
        mov ax, str_error_mmap
        jmp .fatal_error

    .error_sysinfo:
        mov ax, str_error_sysinfo
        jmp .fatal_error

    .fatal_error:
        push ax
        call puts
        add sp, 2

    .halt:
        cli
        hlt
        jmp .halt

%include "bootstrap16/a20.inc"
%include "bootstrap16/cpu.inc"
%include "bootstrap16/gdt.inc"
%include "bootstrap16/mmap.inc"
%include "bootstrap16/nmi.inc"
%include "bootstrap16/stdio.inc"
%include "bootstrap16/strings.inc"
%include "bootstrap16/sysinfo.inc"
%include "bootstrap16/vbe.inc"
