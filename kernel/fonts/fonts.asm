section .rodata

align 8

global g_font16
g_font16:
    dd 16

s_font16Data:
    incbin "fonts/16.bin"

align 8

global g_font8
g_font8:
    dd 8

s_font8Data:
    incbin "fonts/8.bin"
