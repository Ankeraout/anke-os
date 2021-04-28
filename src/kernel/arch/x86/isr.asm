bits 32

section .text

%macro DEF_IRQ_HANDLER 1
global irq_handler_%1
irq_handler_%1:
    ; Save context
    push eax
    push ecx
    push edx
    push ebx
    push ebp
    push esi
    push edi

    ; Save segment registers
    mov ax, ds
    push eax

    ; Restore kernel segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Call IRQ handling routine
    push dword %1
    call interrupt_handle
    add esp, 4

    ; Send EOI to PIC
.eoi:
    mov al, 0x20
%if %1 >= 8
    out 0xa0, al
%endif
    out 0x20, al

    ; Restore segment registers
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Restore context
    pop edi
    pop esi
    pop ebp
    pop ebx
    pop edx
    pop ecx
    pop eax

    ; Return from interrupt
    iret
%endmacro

%macro DEF_EXCEPTION_HANDLER 1
global isr_handler_exception_%1
isr_handler_exception_%1:
    mov eax, %1
    jmp isr_handler_exception
%endmacro

extern interrupt_handle

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
    ; Save context
    push eax
    push ecx
    push edx
    push ebx
    push ebp
    push esi
    push edi

    ; Save segment registers
    mov dx, ds
    push edx

    ; Restore kernel segment registers
    mov dx, 0x10
    mov ds, dx
    mov es, dx
    mov fs, dx
    mov gs, dx

    ; Halt the system (todo: change)
    cli
    hlt

    ; Restore segment registers
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Restore context
    pop edi
    pop esi
    pop ebp
    pop ebx
    pop edx
    pop ecx
    pop eax

    ; Return from interrupt
    iret
