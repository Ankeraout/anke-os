bits 16
org 0x0000
cpu 8086

_start:
    mov ax, 0x800
    mov ds, ax
    mov es, ax
    jmp 0x800:main

main:
    mov si, msg_hello
    call puts
    jmp $

; Input:
; SI = message
puts:
    lodsb
    test al, al
    jz puts_ret

    mov ah, 0x0e
    xor bx, bx
    int 0x10

    jmp puts

    puts_ret:
        ret

msg_hello db "Hello, world!", 13, 10, 0
