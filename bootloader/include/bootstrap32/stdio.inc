console_cursor_x    dd 0
console_cursor_y    dd 0

stdio_init:
    xor eax, eax

    mov dx, 0x3d4
    mov al, 0x0e
    out dx, al
    inc dx
    in al, dx
    dec dx
    mov ah, al
    mov al, 0x0f
    out dx, al
    inc dx
    in al, dx

    xor edx, edx
    mov ecx, 80
    div ecx

    mov [console_cursor_y], eax
    mov [console_cursor_x], edx

    ret

; void puts(const char *p_buffer)
puts:
    push ebp
    mov ebp, esp

    push esi
    mov esi, [ebp + 8]

    .loop:
        lodsb
        test al, al
        jz .end

        push eax
        call putc
        add sp, 4

        jmp .loop

.end:
    pop esi
    pop ebp
    ret

; void putc(char p_character)
putc:
    push ebp
    mov ebp, esp

    cmp byte [ebp + 8], 0
    jz .epilogue

    cmp byte [ebp + 8], 13
    jz .cr

    cmp byte [ebp + 8], 10
    jz .lf

    mov eax, 80
    mul dword [console_cursor_y]
    add eax, [console_cursor_x]
    shl eax, 1

    push edi
    mov edi, eax
    mov al, [ebp + 8]
    mov [edi + 0xb8000], al
    mov byte [edi + 0xb8001], 0x07
    pop edi

    inc dword [console_cursor_x]

.check_x:
    cmp dword [console_cursor_x], 80
    jae .new_line

.epilogue:
    call update_cursor
    pop ebp
    ret

.lf:
    mov dword [console_cursor_x], 0

.new_line:
    inc dword [console_cursor_y]

.check_y:
    cmp dword [console_cursor_y], 25
    jae .wrap
    jmp .epilogue

.wrap:
    mov dword [console_cursor_y], 0
    jmp .epilogue

.cr:
    mov dword [console_cursor_x], 0
    jmp .epilogue

update_cursor:
    mov eax, 80
    mul dword [console_cursor_y]
    add eax, [console_cursor_x]
    mov ecx, eax

    mov dx, 0x3d4
    mov al, 0x0f
    out dx, al
    inc dx
    mov al, cl
    out dx, al
    dec dx
    mov al, 0x0e
    out dx, al
    inc dx
    mov al, ch
    out dx, al

    ret
