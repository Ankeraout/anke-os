bits 32

extern irq_wrapper

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

extern kernel_panic

section .text

%assign i 0
%rep 16
    DEF_IRQ_HANDLER i
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
    push dword msg_kernelPanic
    call kernel_panic
    add esp, 4
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popad
    iret

section .rodata
msg_kernelPanic db "Unhandled CPU exception", 13, 10, 0
