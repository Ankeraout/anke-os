cpu 386

mmap_init:
    push bp
    mov bp, sp
    push es
    push di
    push bx

    ; Store the memory map at 0x2000
    xor ax, ax
    mov es, ax
    mov di, 0x2000
    xor ebx, ebx

    ; Zero out memory
    mov cx, 0x800
    repz stosw

    mov di, 0x2000
    
.loop:
    mov eax, 0xe820
    mov ecx, 24
    mov edx, 0x534d4150
    int 0x15

    jc .loop_end

    add di, 24

    cmp eax, 0x534d4150
    jnz .loop_end

    test ebx, ebx
    jz .loop_end

    ; Stop if the map size is >= 170 entries
    cmp di, 0x8ff0
    je .loop_end

    jmp .loop

.loop_end:
    ; Store the size of the map at 0x8ff0
    sub di, 0x2000
    mov [es:0x2ff0], di

    pop bx
    pop di
    pop es
    pop bp
    ret

cpu 8086
