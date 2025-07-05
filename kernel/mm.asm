; 640 KB / 16 bytes per segment / 8 bits per byte = 5120 bytes
%define C_MM_BITS_PER_BYTE 8
%define C_MM_SEGMENT_SIZE_BYTES 16
%define C_MM_LOW_MEMORY_SIZE_BYTES 655360
%define C_MM_TOTAL_SEGMENT_COUNT (C_MM_LOW_MEMORY_SIZE_BYTES / C_MM_SEGMENT_SIZE_BYTES)
%define C_MM_MMAP_SIZE (C_MM_TOTAL_SEGMENT_COUNT / C_MM_BITS_PER_BYTE)

section .text
; int mm_init()
mm_init:
    push bp
    mov bp, sp
    
    ; Mark all the memory as free
    xor ax, ax
    push ax
    mov dx, C_MM_TOTAL_SEGMENT_COUNT
    push dx
    push ax
    call mm_mark
    add sp, 6

    ; Mark 0x00000-0x00500 as used (IVT + BDA)
    mov ax, 1
    push ax
    mov ax, 0x50
    push ax
    xor ax, ax
    push ax
    call mm_mark
    add sp, 6

    ; If the kernel is in high memory, do not mark as used.
    mov ax, cs
    cmp ax, 0xffff
    jz .skipKernelMarking

    ; Mark the kernel as used
    mov ax, 1
    push ax

    ; Convert the kernel size to segments
    mov ax, g_kernel_end
    add ax, 15
    rcr ax, 1
    mov cl, 3
    shr ax, cl
    push ax

    ; Get kernel first segment
    mov ax, cs
    mov cl, 4
    shr ax, cl
    push ax
    call mm_mark
    add sp, 6

    .skipKernelMarking:
        ; Get the amount of memory from BIOS
        int 0x12

        ; Convert KB to segments
        mov cl, 6
        shl ax, cl

        mov dx, 1
        push dx

        ; Determine the number of unusable segments in memory.
        mov dx, C_MM_TOTAL_SEGMENT_COUNT
        sub dx, ax
        push dx

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
        mov dx, 1
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

section .bss
align 2, resb 1
g_mm_bitmap resb C_MM_MMAP_SIZE
