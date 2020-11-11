bits 32

extern kernel_panic

section .text

global isr_handler0_7
isr_handler0_7:
    push eax
    mov al, 0x20
    out 0x20, al
    pop eax
    iret

global isr_handler8_15
isr_handler8_15:
    push eax
    mov al, 0x20
    out 0xa0, al
    out 0x20, al
    pop eax
    iret

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
