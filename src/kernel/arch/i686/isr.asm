bits 32

%macro DEF_IRQ_HANDLER 1
global isr_handler_%1
isr_handler_%1:
    pushad
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, [isr_irqHandlers + %1 * 4]
    test eax, eax
    jz .eoi

    push dword [isr_irqHandlerParameters + %1 * 4]
    call [isr_irqHandlers + %1 * 4]

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

global isr_registerIRQHandler
%define arg_irq (ebp + 8)
%define arg_handler (ebp + 12)
%define arg_arg (ebp + 16)
isr_registerIRQHandler:
    push ebp
    mov ebp, esp

    mov ecx, [arg_irq]
    shl ecx, 2
    lea eax, [isr_irqHandlers + ecx]
    mov edx, [arg_handler]
    mov [eax], edx
    lea eax, [isr_irqHandlerParameters + ecx]
    mov edx, [arg_arg]
    mov [eax], edx

    pop ebp
    ret
%undef arg_irq
%undef arg_handler
%undef arg_arg

section .rodata
msg_kernelPanic db "Unhandled CPU exception", 13, 10, 0

section .data
isr_irqHandlers:
    times 16 dd 0
isr_irqHandlerParameters:
    times 16 dd 0
