bits 16
cpu 8086

_start:
    mov ax, 0x1000
    mov ds, ax
    mov es, ax
    mov ax, 0x8f00
    mov ss, ax
    xor sp, sp

    push es
    mov ax, 0xb800
    mov es, ax
    mov byte [es:0], 'A'
    mov byte [es:1], 0x1f
    pop es

halt:
    cli
    hlt
    jmp halt
    