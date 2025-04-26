bits 16

_start:
    mov ax, 0x1000
    mov ds, ax
    mov es, ax
    mov ax, 0x7000
    mov ss, ax
    xor sp, sp

    ; Print "Hello, World!" to the screen
    mov si, hello_world
    call print_string

    ; Infinite loop to prevent the CPU from executing random code
.loop:
    ; Halt the CPU
    cli
    hlt
    jmp .loop

print_string:
    mov ah, 0x0e

.next_char: 
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .next_char

.done:
    ret

hello_world db 'Hello, World!', 0
