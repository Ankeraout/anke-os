section .text
bits 64

extern main

global main64
main64:
    ; Prepare stack
    mov rsp, 0x9f000

    ; Call C code
    call main

    ; Halt if main() returns.
halt:
    cli
    hlt
    jmp halt
