%define C_VBE_MAX_WIDTH 800
%define C_VBE_MAX_HEIGHT 600

; int vbe_init(uint8_t p_vbeModeInfoBuffer[256])
;
; Input (stack):
; - [bp + 4]: p_vbeModeInfoBuffer.offset
; - [bp + 6]: p_vbeModeInfoBuffer.segment
;
; Output:
; - ax: video mode (0xffff = error)
;
; Local variables:
; - [bp - 512]: uint8_t l_vbeInfo
vbe_init:
    push bp
    mov bp, sp
    sub sp, 512

    push ss
    lea ax, [bp - 512]
    push ax
    call vbe_get_info
    add sp, 4

    test ax, ax
    jnz .error

    ; Get best video mode
    push ss
    lea ax, [bp - 512]
    push ax

    call vbe_get_best_mode
    add sp, 4

    cmp ax, 0xffff
    jz .error

    ; Set video mode
    push ax
    call vbe_set_mode
    add sp, 2

    test ax, ax
    jnz .error

    add sp, 512
    pop bp
    ret

.error:
    mov ax, 0xffff
    add sp, 512
    pop bp
    ret

; int vbe_get_info(uint8_t p_vbeInfoBuffer[512])
;
; Input (stack):
; - [bp + 4]: p_vbeInfoBuffer.offset
; - [bp + 6]: p_vbeInfoBuffer.segment
;
; Output:
; - ax: return code (0 = success, any other code = error)
vbe_get_info:
    push bp
    mov bp, sp
    push es
    push di

    mov ax, [bp + 6]
    mov es, ax
    mov di, [bp + 4]
    mov ax, 0x4f00
    int 0x10

    pop di
    pop es
    pop bp

    cmp ax, 0x004f
    jnz .error

    xor ax, ax
    ret

.error:
    mov ax, 1
    ret

; int vbe_get_mode_info(uint16_t p_mode, uint8_t p_vbeModeInfoBuffer[256])
;
; Input (stack):
; - [bp + 4]: p_vbeModeInfoBuffer.offset
; - [bp + 6]: p_vbeModeInfoBuffer.segment
; - [bp + 8]: p_mode
;
; Output:
; - ax: return code (0 = success, any other code = error)
vbe_get_mode_info:
    push bp
    mov bp, sp

    push es
    push di

    mov ax, [bp + 6]
    mov es, ax
    mov di, [bp + 4]
    mov cx, [bp + 8]
    mov ax, 0x4f01
    int 0x10

    pop di
    pop es
    pop bp

    cmp ax, 0x004f
    jnz .error

    xor ax, ax
    ret

.error:
    mov ax, 1
    ret

; uint32_t vbe_get_mode_score(const uint8_t p_vbeModeInfoBuffer[256])
;
; Input (stack):
; - [bp + 4]: p_vbeModeInfoBuffer.offset
; - [bp + 6]: p_vbeModeInfoBuffer.segment
;
; Output:
; - dx:ax: mode score (dx = bits 16 to 31, ax = bits 0 to 15)
vbe_get_mode_score:
    push bp
    mov bp, sp
    push es
    push si

    mov ax, [bp + 6]
    mov es, ax
    mov si, [bp + 4]

    ; Check limits
    cmp word [es:si + 0x12], C_VBE_MAX_WIDTH
    ja .unsupported_mode
    cmp word [es:si + 0x14], C_VBE_MAX_HEIGHT
    ja .unsupported_mode

    mov ax, [es:si + 0x12]
    mul word [es:si + 0x14]
    mov dh, [es:si + 0x19]
    
    pop si
    pop es
    pop bp
    ret

.unsupported_mode:
    pop si
    pop es
    pop bp
    xor ax, ax
    xor dx, dx
    ret

; uint16_t vbe_get_best_mode(const uint8_t p_vbeInfo[256])
;
; Input (stack):
; - [bp + 4]: p_vbeInfoBuffer.offset
; - [bp + 6]: p_vbeInfoBuffer.segment
;
; Output:
; - ax: video mode (0xffff = error)
;
; Local variables:
; - |bp - 2]: saved ES
; - [bp - 4]: saved SI
; - [bp - 260]: uint8_t l_vbeModeInfoBuffer[256]
; - [bp - 262]: uint16_t l_bestModeScoreH
; - [bp - 264]: uint16_t l_bestModeScoreL
; - [bp - 266]: uint16_t l_bestMode
; - [bp - 268]: uint16_t l_currentMode
vbe_get_best_mode:
    push bp
    mov bp, sp
    push es
    push si

    sub sp, 264

    ; Initialize local variables
    xor ax, ax
    mov [bp - 262], ax
    mov [bp - 264], ax
    not ax
    mov [bp - 266], ax

    ; Get video mode table segment and offset
    mov ax, [bp + 6]
    mov es, ax
    mov si, [bp + 4]
    mov dx, [es:si + 0xe]
    mov ax, [es:si + 0x10]
    mov es, ax
    mov si, dx
    
.loop:
    ; Read video mode
    mov ax, [es:si]
    mov [bp - 268], ax
    add si, 2

    cmp ax, 0xffff
    jz .loop_end

    ; Get video mode info
    push ax
    push ss
    lea ax, [bp - 260]
    push ax
    call vbe_get_mode_info

    ; Get mode score
    call vbe_get_mode_score

    add sp, 6

    cmp dx, [bp - 262]
    ja .better_mode
    jnae .loop

    cmp ax, [bp - 264]
    ja .better_mode
    jmp .loop

.better_mode:
    mov [bp - 262], dx
    mov [bp - 264], ax
    mov ax, [bp - 268]
    mov [bp - 266], ax
    jmp .loop

.loop_end:
    mov ax, [bp - 266]
    add sp, 264
    pop si
    pop es
    pop bp
    ret

; int vbe_set_mode(uint16_t p_mode)
;
; Input (stack):
; - |bp + 4]: uint16_t p_mode
;
; Output:
; - ax: return value (0 = success, any other value = error)
vbe_set_mode:
    push bp
    mov bp, sp
    push bx

    mov ax, 0x4f02
    mov bx, [bp + 4]
    int 0x10

    pop bx
    pop bp
    cmp ax, 0x004f
    jnz .error
    xor ax, ax
    ret

.error:
    mov ax, 1
    ret

; uint16_t vbe_get_mode()
;
; Output:
; - ax: video mode (0xffff = error)
vbe_get_mode:
    push bx
    mov ax, 0x4f03
    int 0x10

    cmp ax, 0x004f
    jnz .error

    mov ax, bx
    pop bx
    ret

.error:
    mov ax, 0xffff
    pop bx
    ret
