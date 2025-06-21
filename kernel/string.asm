section .text

; char *strrev(char *p_string)
strrev:
    %define p_stringOffset (bp + 4)
    %define p_stringSegment (bp + 6)

    push bp
    mov bp, sp

    push si
    push es
    push di

    push word [p_stringSegment]
    push word [p_stringOffset]
    call strlen
    add sp, 4

    test ax, ax
    jz .end

    dec ax

    mov si, [p_stringOffset]
    mov di, si
    add di, ax
    mov es, [p_stringSegment]

    .loop:
        cmp si, di
        jae .end

        mov al, [es:si]
        xchg al, [es:di]
        mov [es:si], al
        inc si
        dec di

        jmp .loop

    .end:
        pop di
        pop es
        pop si
        pop bp
        ret

    %undef p_stringSegment
    %undef p_stringOffset

; size_t strlen(const char *p_string)
strlen:
    %define p_stringOffset (bp + 4)
    %define p_stringSegment (bp + 6)

    push bp
    mov bp, sp
    
    push es
    push si

    mov es, [p_stringSegment]
    mov si, [p_stringOffset]

    xor ax, ax
    xor cx, cx
    repne scasb
    mov ax, cx
    neg ax

    pop si
    pop es

    pop bp
    ret

    %undef p_stringSegment
    %undef p_stringOffset

; void *memcpy(void *p_dst, const void *p_src, size_t p_size)
memcpy:
    %define p_dstOffset (bp + 4)
    %define p_dstSegment (bp + 6)
    %define p_srcOffset (bp + 8)
    %define p_srcSegment (bp + 10)
    %define p_size (bp + 12)

    push bp
    mov bp, sp

    push ds
    push si
    push es
    push di

    mov es, [p_dstSegment]
    mov di, [p_dstOffset]

    mov ax, [p_srcSegment]
    mov ds, ax
    mov si, [p_srcOffset]

    mov cx, [p_size]

    rep movsb

    pop di
    pop es
    pop si
    pop ds
    pop bp
    ret

    %undef p_dstSegment
    %undef p_dstOffset
    %undef p_srcSegment
    %undef p_srcOffset
    %undef p_size

; void *memset(void *p_dst, int p_value, size_t p_size)
memset:
    %define p_dstOffset (bp + 4)
    %define p_dstSegment (bp + 6)
    %define p_value (bp + 8)
    %define p_size (bp + 10)

    push bp
    mov bp, sp

    push si
    push es
    push di

    les di, [p_dstOffset]

    mov cx, [p_size]
    mov al, [p_value]

    rep stosb

    pop di
    pop es
    pop si
    pop bp
    ret

    %undef p_dstSegment
    %undef p_dstOffset
    %undef p_value
    %undef p_size
