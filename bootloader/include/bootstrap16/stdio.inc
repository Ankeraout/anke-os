; int puts(const char *p_str)
;
; Summary:
; Prints the given text to the screen using BIOS routines.
;
; Input:
; [bp + 4] = offset(p_str)
;
; Returns:
; An integer that indicates the result of the operation:
; - A positive number on success.
; - EOF (-1) on error.
puts:
    push bp
    mov bp, sp

    push si
    mov si, [bp + 4]

    .loop:
        lodsb
        test al, al
        jz .loop_end

        mov ah, 0x0e
        int 0x10

        jmp .loop

    .loop_end:
        pop si
        pop bp
        xor ax, ax
        ret
