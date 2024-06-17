putchar:
%define p_character bp + 4
    push bp
    mov bp, sp

    push bx

    mov al, [p_character]
    mov ah, 0x0e
    xor bx, bx
    int 0x10

    pop bx

    pop bp
    ret
%undef p_character

puts:
%define p_str bp + 4
    push bp
    mov bp, sp
    push si

    mov si, [p_str]

    .loop:
        lodsb
        test al, al
        jz .loop_end
        
        push ax
        call putchar
        add sp, 2

        jmp .loop

    .loop_end:
    pop si
    pop bp
    ret
%undef p_str
