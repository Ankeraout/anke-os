bits 32

section .text

global syscall_wrapper
extern syscall
syscall_wrapper:
    pushad
    push ebx
    push eax

    call syscall

    add esp, 8
    popad
    iret
