bits 16

section .text

; Description:
; Checks if the A20 gate is enabled, and enables it if it is not enabled.
a20_enable:
    push ax
    push bx
    push si

    mov si, .str_checking
    mov bl, 0x07
    call text16_puts

    mov ax, 0x2402
    int 0x15

    jc .error1

    test ah, ah
    jnz .enabled

    mov si, .str_disabled
    mov bl, 0x07
    call text16_puts

    mov ax, 0x2401
    int 0x15
    jc .error2

    mov si, .str_ok
    mov bl, 0x0a
    call text16_puts
    jmp .return

.enabled:
    mov si, .str_error1
    mov bl, 0x0a
    call text16_puts
    jmp .return

.error1:
    mov si, .str_error1
    mov bl, 0x0c
    call text16_puts
    jmp halt16

.error2:
    mov si, .str_error2
    mov bl, 0x0c
    call text16_puts
    jmp halt16

.return:
    pop si
    pop bx
    pop ax
    ret

section .rodata
.str_checking: db "a20: Checking A20 gate... ", 0
.str_disabled: db "Disabled.", 13, 10, "a20: Enabling A20 gate... ", 0
.str_enabled: db "Enabled.", 13, 10, 0
.str_error1: db "Failed to get A20 gate status.", 13, 10, 0
.str_error2: db "Failed to enable A20 gate.", 13, 10, 0
.str_ok: db "OK", 13, 10, 0
