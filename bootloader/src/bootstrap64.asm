section .text
bits 64

extern main
extern __bss_start
extern __bss_end
extern memset

global main64
main64:
    ; Prepare stack
    mov rsp, 0x9f000

    ; Clear .bss
    mov rdi, __bss_start
    mov rdx, __bss_end
    sub rdx, rdi
    xor rsi, rsi

    ; Pass the pointer to the system information structure as a parameter
    ; NOTE: this will clear the 32 upper bits of RDI
    mov edi, ebx

    ; Call C code
    call main

    ; Halt if main() returns.
halt:
    cli
    hlt
    jmp halt
