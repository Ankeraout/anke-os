section .text
; void printk(const char *p_str);
printk:
    %define p_strSegment (bp + 4)
    %define p_strOffset (bp + 6)

    push bp
    mov bp, sp

    push ds
    push si

    mov ax, [p_strSegment]
    mov ds, ax

    mov si, [p_strOffset]

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

    %undef p_strSegment
    %undef p_strOffset
