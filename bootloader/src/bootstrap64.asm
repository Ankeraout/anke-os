section .text
bits 64

extern main
extern puts

global main64
main64:
    ; Prepare stack
    mov rsp, 0x9f000

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
