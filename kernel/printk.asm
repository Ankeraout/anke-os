section .text
printk:
    push bp
    mov bp, sp

    push ds
    push si

    mov ax, [bp + 4]
    mov ds, ax

    mov si, [bp + 6]

    mov ah, 0x0e

    .loop:
        lodsb
        test al, al
        jz .done

        int 0x10
        jmp .loop

    .done:
        pop si
        pop ds
        pop bp
        ret
