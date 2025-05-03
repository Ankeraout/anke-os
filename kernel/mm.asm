; 640 KB / 16 bytes per segment / 8 bits per byte = 5120 bytes
%define C_MM_MMAP_SIZE 5120

section .text
; void mm_init()
mm_init:
    %define l_utoaBuffer (bp - 8)
    push bp
    mov bp, sp
    sub sp, 8
    
    .initializeMap:
        mov ax, C_MM_MMAP_SIZE
        push ax
        mov al, 0xff
        push ax
        mov ax, g_mm_bitmap
        push ax
        push ds
        call memset
        add sp, 8

    .markFreeMemory:
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

    .printDetectedMemory:
        ; Print the detected memory message
        mov ax, g_mm_strDetectedMemory
        push ax
        push ds
        call printk
        add sp, 4
        
        ; Get the amount of memory from BIOS
        int 0x12

        ; Convert the amount of memory to a string
        mov dx, 10
        push dx
        lea dx, [l_utoaBuffer]
        push dx
        push ss
        push ax
        call utoa
        add sp, 8

        ; Print the amount of memory
        lea ax, [l_utoaBuffer]
        push ax
        push ss
        call printk
        add sp, 4

        ; Print the KB suffix
        mov ax, g_mm_strDetectedMemory2
        push ax
        push ds
        call printk
        add sp, 4

    add sp, 8
    pop bp
    ret

    %undef l_utoaBuffer

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

            loop .markLoop
        
        .incrementByteOffset:
            inc di
            loop .markLoop

    pop di
    pop bp
    ret

    %undef p_firstSegment
    %undef p_nbSegments

section .rodata
g_mm_strDetectedMemory db "mm: Detected memory: ", 0
g_mm_strDetectedMemory2 db " KB", 13, 10, 0

section .bss
g_mm_bitmap resb C_MM_MMAP_SIZE
