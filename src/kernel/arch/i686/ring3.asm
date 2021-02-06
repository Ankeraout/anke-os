bits 32

section .text

global ring3_call
ring3_call:
    mov ecx, [esp + 4]
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    mov eax, esp
    push dword 0x23
    push eax
    pushf
    push dword 0x1b
    push ecx
    iret
