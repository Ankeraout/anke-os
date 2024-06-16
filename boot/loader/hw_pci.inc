hw_detect_pci:
%define p_pciInfo bp + 4
    push bp
    mov bp, sp
    push bx
    push di

    mov ax, g_str_hw_detect_pci
    push ax
    call puts
    add sp, 2

    ; If the CPU is not a 386+, we cannot detect the PCI bus using the BIOS as
    ; this requires using 32-bit registers.
    mov al, [g_hw_cpu]
    cmp al, 3
    jge .no_pci

    cpu 386
    mov ax, 0xb101
    xor edi, edi
    int 0x1a

    jc .no_pci

    test ah, ah
    jnz .no_pci

    cmp edx, 0x20494350
    jnz .no_pci

    mov di, [p_pciInfo]
    mov [di], al
    mov [di + 1], bh
    mov [di + 2], bl
    mov [di + 3], cl

    cpu 8086
    mov ax, g_str_hw_detect_pci_version
    push ax
    call puts
    add sp, 2

    mov al, [di + 1]
    shr al, 1
    shr al, 1
    shr al, 1
    shr al, 1
    add al, '0'
    push ax
    call putchar
    add sp, 2
    
    mov al, [di + 1]
    and al, 0x0f
    add al, '0'
    push ax
    call putchar
    add sp, 2

    mov al, '.'
    push ax
    call putchar
    add sp, 2

    mov al, [di + 2]
    shr al, 1
    shr al, 1
    shr al, 1
    shr al, 1
    add al, '0'
    push ax
    call putchar
    add sp, 2
    
    mov al, [di + 2]
    and al, 0x0f
    add al, '0'
    push ax
    call putchar
    add sp, 2

    cpu 8086
    mov ax, g_str_hw_detect_pci_version_end
    push ax
    call puts
    add sp, 2

    jmp .epilogue

.no_pci:
    mov di, [p_pciInfo]
    mov word [di], 0
    mov word [di + 2], 0

    mov ax, g_str_hw_detect_pci_no
    push ax
    call puts
    add sp, 2

.epilogue:
    pop di
    pop bx
    pop bp
    ret
%undef p_pciInfo
