bits 32

section .text

global callUsermode
%define arg_ptr (ebp + 8)
callUsermode:
    push ebp
    mov ebp, esp

    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, esp
    push dword 0x23 ;ss
    push eax ;esp
    pushf ;eflags
    push 0x1b ;cs
    push dword [arg_ptr] ;eip
    iret

    pop ebp
    ret
%undef arg_ptr
