section .rodata

global g_font16
g_font16:
    dq 16
    dq s_font16Data

global g_font8
g_font8:
    dq 8
    dq s_font8Data

s_font16Data:
    incbin "src/fonts/16.bin"

s_font8Data:
    incbin "src/fonts/8.bin"
