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

; void *malloc(uint16_t p_size)
malloc:
    %define p_size (bp + 4)
    
    push bp
    mov bp, sp

    mov ax, [p_size]

    ; If the size is 0, return NULL.
    test ax, ax
    jz .error

    ; If the size is greater than 65518, return NULL.
    cmp ax, 65518
    ja .error

    ; Convert the size to segments. Add 2 bytes for the allocation header.
    add ax, 17
    mov cl, 4
    shr ax, cl

    ; Allocate the memory
    push ax
    call mm_alloc
    add sp, 2

    ; If mm_alloc returned 0, then the allocation failed.
    test ax, ax
    jz .error

    ; Add the allocation header
    push es
    mov es, ax
    mov dx, [p_size]
    mov [es:0x0000], dx
    pop es

    ; Return the pointer to the allocated memory
    mov dx, ax
    mov ax, 2

    .end:
        pop bp
        ret

    .error:
        xor ax, ax
        xor dx, dx
        jmp .end

    %undef p_size

; void free(void *p_ptr)
free:
    %define p_segment (bp + 4)
    %define p_offset (bp + 6)

    push bp
    mov bp, sp

    ; Read the size from the allocation header
    push es
    push di
    mov ax, [p_segment]
    mov es, ax
    mov di, [p_offset]
    mov ax, [es:di]
    pop di
    pop es

    ; Convert the size to segments
    add ax, 17
    mov cl, 4
    shr ax, cl

    ; Free the memory
    xor dx, dx
    push dx
    push ax
    push word [p_segment]
    call mm_mark
    add sp, 6

    pop bp
    ret

    %undef p_segment
    %undef p_offset

section .rodata
g_utoa_baseString db "0123456789abcdefghijklmnopqrstuvwxyz"
