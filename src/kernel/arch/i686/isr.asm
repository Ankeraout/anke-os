bits 32

section .text

%macro DEF_IRQ_HANDLER 1
global irq_handler_%1
irq_handler_%1:
    pushad
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push dword %1
    call irq_wrapper

    add esp, 4

.eoi:
    mov al, 0x20
%if %1 >= 8
    out 0xa0, al
%endif
    out 0x20, al
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popad
    iret
%endmacro

%macro DEF_EXCEPTION_HANDLER 1
global isr_handler_exception_%1
isr_handler_exception_%1:
    mov eax, %1
    jmp isr_handler_exception
%endmacro

extern irq_wrapper
extern printf
extern syscall

section .text

%assign i 0
%rep 16
    DEF_IRQ_HANDLER i
%assign i i + 1
%endrep

%assign i 0
%rep 32
    DEF_EXCEPTION_HANDLER i
%assign i i + 1
%endrep

global isr_handler_exception
isr_handler_exception:
    pushad
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push dword msg_exception
    call printf

    cli
    hlt
    add esp, 4
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popad
    iret

global isr_syscall
isr_syscall:
    push edx
    push ecx

    mov dx, ds
    push edx

    mov dx, 0x10
    mov ds, dx
    mov es, dx
    mov fs, dx
    mov gs, dx

    push ebx
    push eax

    call syscall

    add esp, 8

    pop edx
    mov ds, dx
    mov es, dx
    mov fs, dx
    mov gs, dx

    pop ecx
    pop edx

    iret

section .rodata
msg_exception db "CPU exception %#02x", 10, 0
