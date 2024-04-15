; void puts(const char *p_str)
;
; Input:
; [bp + 4] = const char *p_str
puts:
    push bp
    mov bp, sp

    push si
    mov si, [bp + 4]

    puts_loop:
        lodsb
        test al, al
        jz puts_loop_end

        mov ah, 0x0e
        int 0x10

        jmp puts_loop

puts_loop_end:
    pop si
    pop bp
    ret
