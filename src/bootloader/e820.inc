bits 16

; Description:
; Reads the memory map and puts it at 0x00020000 in memory.
;
; Output:
; The memory map at 0x00020000
e820_readMemoryMap:
    push ax
    push bx
    push cx
    push dx
    push es
    push di

    mov si, .str_readingMemoryMap
    mov bl, 0x07
    call text16_puts

    push es
    mov ax, 0x2000
    mov es, ax
    xor di, di
    xor ebx, ebx

.next:
    mov eax, 0x0000e820
    mov ecx, 24
    mov edx, 0x534d4150
    int 0x15
    jc .error1

    cmp eax, 0x534d4150
    jne .error2

    cmp ebx, 0
    je .ok

    cmp di, 65536 - 24
    jae .error3

    add di, 24
    jmp .next
    
.ok:
    pop es
    mov [memoryMapSize], cx

    mov si, .str_ok
    mov bl, 0x0a
    call text16_puts

    pop di
    pop es
    pop dx
    pop cx
    pop bx
    pop ax
    ret

.error1:
    mov si, .str_error1
    mov bl, 0x0c
    call text16_puts
    jmp halt16

.error2:
    mov si, .str_error1
    mov bl, 0x0c
    call text16_puts
    jmp halt16

.error3:
    mov si, .str_error3
    mov bl, 0x0c
    call text16_puts
    jmp halt16
    
.str_ok: db "OK", 13, 10, 0
.str_readingMemoryMap: db "e820: Reading memory map... ", 0
.str_error1: db "BIOS call failed.", 13, 10, 0
.str_error2: db "Wrong returned signature.", 13, 10, 0
.str_error3: db "The memory map is bigger than 64kB.", 13, 10, 0