bits 32

%macro M_DEFINE_EXCEPTION 1
    global isr_exception%1
    isr_exception%1:
        push 0 ; No error code
        push %1 ; Interrupt number
        jmp isr_common
%endmacro

%macro M_DEFINE_EXCEPTION_ERRCODE 1
    global isr_exception%1
    isr_exception%1:
        push %1 ; Interrupt number
        jmp isr_common
%endmacro

%macro M_DEFINE_IRQ 1
    global isr_irq%1
    isr_irq%1:
        push 0 ; No error code
        push %1 ; Interrupt number
        jmp isr_common
%endmacro

extern isr_handler

section .text
M_DEFINE_EXCEPTION 0
M_DEFINE_EXCEPTION 1
M_DEFINE_EXCEPTION 2
M_DEFINE_EXCEPTION 3
M_DEFINE_EXCEPTION 4
M_DEFINE_EXCEPTION 5
M_DEFINE_EXCEPTION 6
M_DEFINE_EXCEPTION 7
M_DEFINE_EXCEPTION_ERRCODE 8
M_DEFINE_EXCEPTION 9
M_DEFINE_EXCEPTION_ERRCODE 10
M_DEFINE_EXCEPTION_ERRCODE 11
M_DEFINE_EXCEPTION_ERRCODE 12
M_DEFINE_EXCEPTION_ERRCODE 13
M_DEFINE_EXCEPTION_ERRCODE 14
M_DEFINE_EXCEPTION 15
M_DEFINE_EXCEPTION 16
M_DEFINE_EXCEPTION_ERRCODE 17
M_DEFINE_EXCEPTION 18
M_DEFINE_EXCEPTION 19
M_DEFINE_EXCEPTION 20
M_DEFINE_EXCEPTION_ERRCODE 21
M_DEFINE_EXCEPTION 22
M_DEFINE_EXCEPTION 23
M_DEFINE_EXCEPTION 24
M_DEFINE_EXCEPTION 25
M_DEFINE_EXCEPTION 26
M_DEFINE_EXCEPTION 27
M_DEFINE_EXCEPTION 28
M_DEFINE_EXCEPTION_ERRCODE 29
M_DEFINE_EXCEPTION_ERRCODE 30
M_DEFINE_EXCEPTION 31
M_DEFINE_IRQ 32
M_DEFINE_IRQ 33
M_DEFINE_IRQ 34
M_DEFINE_IRQ 35
M_DEFINE_IRQ 36
M_DEFINE_IRQ 37
M_DEFINE_IRQ 38
M_DEFINE_IRQ 39
M_DEFINE_IRQ 40
M_DEFINE_IRQ 41
M_DEFINE_IRQ 42
M_DEFINE_IRQ 43
M_DEFINE_IRQ 44
M_DEFINE_IRQ 45
M_DEFINE_IRQ 46
M_DEFINE_IRQ 47

isr_common:
    ; Save all registers
    push ebx
    push ebp
    push eax
    push ecx
    push edx
    push edi
    push esi
    mov ax, ds
    push eax
    mov ax, es
    push eax
    mov ax, fs
    push eax
    mov ax, gs
    push eax

    push esp ; Parameter: struct ts_registers

    cld ; SysV i386 calling convention: direction flag must be clear

    call isr_handler

    add esp, 4

    ; Restore registers
    pop eax
    mov gs, ax
    pop eax
    mov fs, ax
    pop eax
    mov es, ax
    pop eax
    mov ds, ax
    pop esi
    pop edi
    pop edx
    pop ecx
    pop eax
    pop ebp
    pop ebx

    add esp, 8 ; Pop interrupt number and error code

    iretd
