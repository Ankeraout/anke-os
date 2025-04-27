section .text

; char *utoa(unsigned p_value, char *p_buffer, int p_base)
utoa:
    %define p_value (bp + 4)
    %define p_bufferSegment (bp + 6)
    %define p_bufferOffset (bp + 8)
    %define p_base (bp + 10)

    push bp
    mov bp, sp
    push es
    push di
    push si

    mov ax, [p_bufferSegment]
    mov es, ax
    mov di, [p_bufferOffset]

    ; Make sure that p_base is valid
    cmp word [p_base], 2
    jb .end
    cmp word [p_base], 36
    ja .end

    mov ax, [p_value]

    .loop:
        ; AX = p_value // p_base
        ; DX = p_value % p_base
        xor dx, dx
        div word [p_base]

        ; Store the remainder in the buffer
        mov si, dx
        mov dl, [g_utoa_baseString + si]
        mov byte [es:di], dl
        inc di

        ; Loop until p_value == 0
        test ax, ax
        jnz .loop

    .reverse:
        mov byte [es:di], 0
        push word [p_bufferOffset]
        push es
        call strrev
        add sp, 4

    .end:
        mov dx, [p_bufferSegment]
        mov ax, [p_bufferOffset]
        pop si
        pop di
        pop es
        pop bp
        ret

    %undef p_value
    %undef p_bufferSegment
    %undef p_bufferOffset
    %undef p_base

; char *itoa(int p_value, char *p_buffer, int p_base)
itoa:
    %define p_value (bp + 4)
    %define p_bufferSegment (bp + 6)
    %define p_bufferOffset (bp + 8)
    %define p_base (bp + 10)

    push bp
    mov bp, sp

    push es

    mov ax, [p_bufferSegment]
    mov es, ax
    mov di, [p_bufferOffset]

    ; Make sure that p_base is valid
    cmp word [p_base], 2
    jb .error
    cmp word [p_base], 36
    ja .error

    ; If p_value is negative, add a minus sign and negate it
    mov ax, [p_value]
    test ax, ax

    jns .positive

    mov byte [es:di], '-'
    inc di
    neg ax

    .positive:
        ; Call utoa to convert the number to a string
        push word [p_base]
        push di
        push es
        push ax
        call utoa
        add sp, 8

    .end:
        mov dx, [p_bufferSegment]
        mov ax, [p_bufferOffset]
        pop es
        pop bp
        ret

    .error:
        mov byte [es:di], 0
        jmp .end

    %undef p_value
    %undef p_bufferSegment
    %undef p_bufferOffset
    %undef p_base

section .rodata
g_utoa_baseString db "0123456789abcdefghijklmnopqrstuvwxyz"
