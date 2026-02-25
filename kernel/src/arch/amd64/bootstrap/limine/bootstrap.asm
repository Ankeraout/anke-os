bits 64

extern bootstrap_init

global _start
extern main

section .text

_start:
    ; Initialize kernel stack
    mov rsp, s_kernelStackBottom

    ; Initialize kernel
    call bootstrap_init

    ; Call main
    jmp main

section .bss
align 8

s_kernelStackTop:
    resb 4096
s_kernelStackBottom:
