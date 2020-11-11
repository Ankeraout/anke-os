bits 32

section .text

global syscall_wrapper
extern syscall
syscall_wrapper:
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

global syscall_call
%define arg_function (ebp + 8)
%define arg_argument (ebp + 12)
syscall_call:
    push ebp
    mov ebp, esp

    push ebx

    mov eax, [arg_function]
    mov ebx, [arg_argument]

    int 0x80

    pop ebx

    pop ebp
    ret
