; int a20_enable(void)
;
; Summary:
; Enables the A20 gate.
;
; Returns:
; An integer that indicates the result of the operation:
; - 0 on success,
; - Any other value on error.
a20_enable:
    push bp
    mov bp, sp

    ; If the A20 gate is already enabled, then we have nothing to do.
    call a20_is_enabled
    test ax, ax
    jnz .success

    ; Try to enable the A20 gate with the BIOS.
    call a20_enable_bios

    ; Check if the A20 gate was enabled.
    call a20_is_enabled
    test ax, ax
    jnz .success

.failure:
    mov ax, 1
    pop bp
    ret

.success:
    xor ax, ax
    pop bp
    ret

; void a20_enable_bios(void)
;
; Summary:
; Tries to enable the A20 gate using a BIOS call.
a20_enable_bios:
    mov ax, 0x2401
    int 0x15
    ret

; int a20_is_enabled(void)
;
; Summary:
; Checks if the A20 gate is enabled.
;
; Returns:
; An integer that indicates whether the A20 gate is enabled:
; - 0 if the A20 gate is disabled,
; - Any other value if the A20 gate is enabled.
a20_is_enabled:
    push bp
    mov bp, sp

    ; Write 0xa5a5 at 0x7c00
    push es
    xor ax, ax
    mov es, ax
    mov word [es:0x7c00], 0xa5a5
    
    ; Write 0x5a5a at 0x107c00
    mov ax, 0xffff
    mov es, ax
    mov word [es:0x7c10], 0x5a5a

    ; If the value at 0x7c00 is 0xa5a5, then the A20 gate is enabled.
    xor ax, ax
    mov es, ax
    mov ax, [es:0x7c00]
    pop es

    cmp ax, 0xa5a5
    jz .enabled

.disabled:
    mov ax, 0
    jmp .epilogue

.enabled:
    mov ax, 1

.epilogue:
    pop bp
    ret
