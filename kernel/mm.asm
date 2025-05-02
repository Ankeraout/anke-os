; 640 KB / 16 bytes per segment / 8 bits per byte = 5120 bytes
%define C_MM_MMAP_SIZE 5120

section .text
mm_init:
    %define l_utoaBuffer (bp - 8)
    push bp
    mov bp, sp
    sub sp, 8

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

    ; TODO: Mark 0x500-0xffff as free
    ; TODO: Mark 0x20000-0x????? as free

    add sp, 8
    pop bp
    ret

    %undef l_utoaBuffer

g_mm_strDetectedMemory db "mm: Detected memory: ", 0
g_mm_strDetectedMemory2 db " KB", 13, 10, 0

section .bss
g_mm_bitmap resb C_MM_MMAP_SIZE
