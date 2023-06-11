section .rodata

align 8

global g_font8
g_font8:
    dd 8

s_font8Data:
    incbin "fonts/8.bin"
