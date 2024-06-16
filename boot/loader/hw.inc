hw_detect:
    push bp
    mov bp, sp

    mov ax, g_str_hw_detect
    push ax
    call puts
    add sp, 2

    call hw_detect_cpu
    mov [g_hw_cpu], al

    mov ax, g_hw_pci
    push ax
    call hw_detect_pci
    add sp, 2

    pop bp
    ret

g_hw_cpu: db 0
g_hw_pci: times 4 db 0
