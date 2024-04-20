cpu x64

; Initialize the system information structure
sysinfo_init:
    push bp
    mov bp, sp

    push es
    push di
    push bx

    xor ax, ax
    mov es, ax

    mov di, 0x1000
    mov cx, 0x800
    repz stosw
    
    mov byte [es:0x1000], 0x01          ; Boot protocol version 1
    mov dword [es:0x1008], str_bootloader_version + 0x10000
    mov word [es:0x1010], 0x2000        ; Memory map at 0x0000000000002000
    mov di, [es:0x2ff0]                 ; Memory map size (copy from 0x2ff0)
    mov [es:0x1018], di

    call sysinfo_pci
    call sysinfo_disk
    call sysinfo_framebuffer
    call sysinfo_bda
    
    pop bx
    pop di
    pop es
    pop bp

    xor ax, ax

    ret

sysinfo_pci:
    mov ax, 0xb101
    xor edi, edi
    int 0x1a
    jc .not_supported
    test ah, ah
    jnz .not_supported
    cmp edx, 0x20494350
    jnz .not_supported

    mov byte [es:0x1020], 1 ; m_pciSupported
    mov [es:0x1021], al ; m_pciFlags
    mov [es:0x1022], bh ; m_pciVersion[0]
    mov [es:0x1023], bl ; m_pciVersion[1]
    mov [es:0x1024], cl ; m_lastPciBus

    ret

.not_supported:
    mov byte [es:0x1020], 0 ; m_pciSupported
    ret

sysinfo_disk:
    ; TODO: how to get these?
    ret

sysinfo_framebuffer:
    push bp
    mov bp, sp

    sub sp, 256

    call vbe_get_mode

    push ax
    push ss
    lea ax, [bp - 256]
    push ax

    call vbe_get_mode_info

    add sp, 6

    test ax, ax
    jnz .error

    mov byte [es:0x1030], 1
    mov byte [es:0x1031], 0
    mov ax, [bp - 256 + 0x28]
    mov [es:0x1038], ax
    mov ax, [bp - 256 + 0x2a]
    mov [es:0x103a], ax
    mov ax, [bp - 256 + 0x12]
    mov [es:0x1040], ax
    mov ax, [bp - 256 + 0x14]
    mov [es:0x1048], ax
    mov ax, [bp - 256 + 0x10]
    mov [es:0x1050], ax
    mov al, [bp - 256 + 0x19]
    mov [es:0x1058], al
    mov al, [bp - 256 + 0x20]
    mov [es:0x1059], al
    mov al, [bp - 256 + 0x1f]
    mov [es:0x105a], al
    mov al, [bp - 256 + 0x22]
    mov [es:0x105b], al
    mov al, [bp - 256 + 0x21]
    mov [es:0x105c], al
    mov al, [bp - 256 + 0x24]
    mov [es:0x105d], al
    mov al, [bp - 256 + 0x23]
    mov [es:0x105e], al

    add sp, 256
    pop bp
    ret

.error:
    mov byte [es:0x1030], 0
    mov byte [es:0x1031], 1
    add sp, 256
    pop bp
    ret

sysinfo_bda:
    push ds
    push si

    xor ax, ax
    mov ds, ax

    mov si, 0x400
    mov cx, 7
    mov di, 0x1060
    repz movsw

    pop si
    pop ds

    ret
