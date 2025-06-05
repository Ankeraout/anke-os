; 640 KB / 16 bytes per segment / 8 bits per byte = 5120 bytes
%define C_MM_MMAP_SIZE 5120

section .text
; int mm_init()
mm_init:
    push bp
    mov bp, sp
    
    ; Mark all the memory as used
    mov ax, C_MM_MMAP_SIZE
    push ax
    mov al, 0xff
    push ax
    mov ax, g_mm_bitmap
    push ax
    push ds
    call memset
    add sp, 8

    ; Mark 0x500-0xffff as free
    xor ax, ax
    push ax
    mov ax, (0x10000 - 0x500) >> 4
    push ax
    mov ax, 0x50
    push ax
    call mm_mark
    add sp, 6

    ; Get the amount of memory from BIOS
    int 0x12

    ; Convert KB to segments
    mov cl, 6
    shl ax, cl

    ; Mark 0x20000-0x((ax << 4) + 0x0f) as free
    xor dx, dx
    push dx
    sub ax, 0x2000
    push ax
    mov ax, 0x2000
    push ax
    call mm_mark
    add sp, 6

    xor ax, ax

    pop bp
    ret

; void mm_mark(uint16_t p_firstSegment, uint16_t p_nbSegments, bool p_used)
mm_mark:
    %define p_firstSegment (bp + 4)
    %define p_nbSegments (bp + 6)
    %define p_used (bp + 8)

    push bp
    mov bp, sp
    push di

    ; Compute DL = mark mask
    mov di, [p_firstSegment]
    mov cx, di
    and cl, 0x07
    mov dl, 0x01
    rol dl, cl

    .computeByteOffset:
        ; Compute the byte offset in DI
        mov cl, 3
        shr di, cl
        add di, g_mm_bitmap

        ; Set CX to the number of segments to mark
        mov cx, [p_nbSegments]

    .markLoop:
        ; Get the current byte
        mov al, [di]

        ; Set or clear the bit
        cmp word [p_used], 0
        jnz .setBit

        .clearBit:
            mov dh, dl
            not dh
            and al, dh
            jmp .storeByte

        .setBit:
            or al, dl
        
        .storeByte:
            ; Store the byte back
            mov [di], al

            ; Increment the bit offset
            rol dl, 1

            ; Increment the byte offset
            cmp dl, 1
            jz .incrementByteOffset

        .keepLooping:
            loop .markLoop
            jmp .end
        
        .incrementByteOffset:
            inc di
            jmp .keepLooping

    .end:
        pop di
        pop bp
        ret

    %undef p_firstSegment
    %undef p_nbSegments

; uint16_t mm_alloc(uint16_t p_nbSegments)
mm_alloc:
    %define p_nbSegments (bp + 4)    
    
    push bp
    mov bp, sp
    push di

    mov di, g_mm_bitmap
    mov cx, C_MM_MMAP_SIZE << 3
    xor dx, dx
    mov ah, 0x01

    .allocLoop:
        ; Get the current byte in the bitmap
        mov al, [di]

        ; Check if the current bit is free
        test al, ah
        jnz .notFree

        ; If the current bit is free, increment the free segment counter
        inc dx

        ; Check if we have found enough free segments
        cmp dx, [p_nbSegments]
        jz .found

        .keepSearching:
            ; If we haven't found enough free segments, continue searching
            rol ah, 1

            ; Increment the byte offset
            cmp ah, 1
            jz .incrementByteOffset

        .keepLooping:
            loop .allocLoop

        .notFound:
            xor ax, ax
            jmp .end

        .incrementByteOffset:
            inc di
            jmp .keepLooping

        .notFree:
            ; If the current bit is not free, reset the free segment counter
            xor dx, dx

            ; Continue searching for free segments
            jmp .keepSearching

    .found:
        ; Compute the bit offset
        xor dx, dx
        shr ah, 1
        jc .computeSegmentOffset
        inc dx
        shr ah, 1
        jc .computeSegmentOffset
        inc dx
        shr ah, 1
        jc .computeSegmentOffset
        inc dx
        shr ah, 1
        jc .computeSegmentOffset
        inc dx
        shr ah, 1
        jc .computeSegmentOffset
        inc dx
        shr ah, 1
        jc .computeSegmentOffset
        inc dx
        shr ah, 1
        jc .computeSegmentOffset
        inc dx

    .computeSegmentOffset:
        ; Compute the address of the first free segment
        ; Compute offset from byte index
        mov ax, di
        sub ax, g_mm_bitmap
        mov cl, 3
        shl ax, cl

        ; Add offset from bit index
        add ax, dx

        ; Subtract segment count
        sub ax, [p_nbSegments]
        inc ax

        ; Mark the allocated segments as used
        push ax
        mov dl, 1
        push dx
        push word [p_nbSegments]
        push ax
        call mm_mark
        add sp, 6
        pop ax

    .end:
        pop di
        pop bp
        ret

    %undef p_nbSegments

section .rodata
g_mm_strDetectedMemory db "mm: Detected memory: ", 0
g_mm_strDetectedMemory2 db " KB", 13, 10, 0

section .bss
g_mm_bitmap resb C_MM_MMAP_SIZE
