; Input:
; SI = message
puts:
    lodsb
    test al, al
    jz .ret

    mov ah, 0x0e
    xor bx, bx
    int 0x10

    jmp puts

    .ret:
        ret
