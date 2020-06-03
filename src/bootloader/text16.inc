bits 16

section .text
; Input:
;  - AL: character to print
;  - BL: color
text16_putc:
    push ax
    push bx

    mov ah, 0x0e
    xor bh, bh
    int 0x10

    pop bx
    pop ax
    ret

; Input:
;  - DS:SI: offset to string
;  - BL: color
text16_puts:
    push ax
    push bx
    push ds
    push si

    xor bh, bh

.loop:
    lodsb
    test al, al
    jz .loop_end
    call text16_putc
    jmp .loop

.loop_end:
    pop si
    pop ds
    pop bx
    pop ax

    ret
