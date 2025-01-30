bits 64

global _start
extern main

section .text

_start:
    ; Initialize kernel stack
    mov rsp, s_kernelStackBottom

    ; Call main
    jmp main

section .bss
s_kernelStackTop:
    resb 4096
s_kernelStackBottom:
