bits 32

section .data
text32_width dd 80
text32_height dd 25
text32_cursorX dd 0
text32_cursorY dd 0

section .text

; Description:
; Retrieves the coordinates of the cursor.
text32_init:
    push eax
    push ebx
    push ecx
    push edx

    ; Get low part
    mov dx, 0x3d4
    mov al, 0x0f
    out dx, al
    inc dx
    in al, dx
    mov cl, al

    ; Get low part
    dec dx
    mov al, 0x0e
    out dx, al
    inc dx
    in al, dx
    mov ah, al
    mov al, cl
    and eax, 0x0000ffff

    ; Compute coordinates
    xor edx, edx
    mov ecx, [text32_width]
    div ecx

    ; Save coordinates
    mov [text32_cursorY], eax
    mov [text32_cursorX], edx

    pop edx
    pop ecx
    pop ebx
    pop eax

    ret

; Description:
; Prints a character to the screen in teletype mode.
;
; Input:
;  - AL: The character to print
;  - BL: The color of the character
text32_putc:
    push eax
    push ebx
    push ecx
    push edx

    cmp al, 0x09
    je .tab

    cmp al, 10
    je .newline

    cmp al, 13
    je .cr

    push eax
    mov eax, [text32_cursorY]
    mul dword [text32_width]
    add eax, [text32_cursorX]
    shl eax, 1
    pop edx
    add eax, 0xb8000
    mov [eax], dl
    mov [eax + 1], bl

    ; Increment the X position
    inc dword [text32_cursorX]

.checkX:
    mov eax, [text32_cursorX]
    cmp eax, [text32_width]
    jge .newline

.checkY:
    mov eax, [text32_cursorY]
    cmp eax, [text32_height]
    jge .checkY_scrollup

.return:
    pop edx
    pop ecx
    pop ebx
    pop eax

    ret

.tab:
    mov edx, [text32_cursorX]
    and edx, 4
    mov eax, 4
    sub eax, edx
    add [text32_cursorX], eax
    jmp .checkX

.cr:
    mov dword [text32_cursorX], 0
    jmp .return

.newline:
    mov dword [text32_cursorX], 0
    inc dword [text32_cursorY]
    jmp .checkY

.checkY_scrollup:
    mov eax, [text32_height]
    sub eax, [text32_cursorY]
    call text32_scrollup
    jmp .return

; ESI: pointer to the string
; BL: color
text32_puts:
    push esi

.loop:
    lodsb
    test al, al
    jz .end
    call text32_putc
    jmp .loop

.end:
    pop esi
    ret

; EAX: number of rows
text32_scrollup:
    push esi
    push edi
    push eax
    push ecx
    push edx

    ; If EAX is greater than the screen height - 1, do a cls
    mov ecx, [text32_height]
    dec ecx
    cmp eax, ecx
    jge .cls

    ; Scroll the cursor
    sub [text32_cursorX], eax

    push eax

    ; edx = number of bytes in a video page
    mov eax, [text32_width]
    shl eax, 1
    mul dword [text32_height]
    mov edx, eax

    ; eax = number of bytes to fill with zeros
    pop eax
    shl eax, 1
    push edx
    mul dword [text32_width]
    pop edx

    ; ecx = number of bytes to copy
    mov ecx, edx
    sub ecx, eax

    ; esi = pointer to the first byte to copy
    mov esi, 0xb8000
    add esi, eax

    ; edi = pointer to the start of the buffer
    mov edi, 0xb8000
    
    ; Copy the bytes
    shr ecx, 2
    repz movsd

    ; Fill the remaining bytes with zeros
    mov ecx, edx
    shr ecx, 2
    xor eax, eax
    repz stosd

.return:
    pop edx
    pop ecx
    pop eax
    pop edi
    pop esi
    ret

.cls:
    call text32_cls
    jmp .return

text32_cls:
    push eax
    push ecx
    push edi
    mov edi, 0xb8000
    mov eax, [text32_width]
    mul dword [text32_height]
    mov ecx, eax
    shr ecx, 1
    repz stosd
    pop edi
    pop ecx
    pop eax
    ret
