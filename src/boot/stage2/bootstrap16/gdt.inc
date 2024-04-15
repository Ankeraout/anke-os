cpu 286

gdt:
    .null   times 8 db 0
    .cs     db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9b, 0xcf, 0x00
    .ds     db 0xff, 0xff, 0x00, 0x00, 0x00, 0x93, 0xcf, 0x00
    .end:

gdtr:
    .limit  dw gdt.end - gdt - 1
    .base   dd gdt + 0x00010000

gdt_init:
    push bp
    sub sp, 6

    mov word [bp - 6], gdt.end - gdt
    mov word [bp - 4], 0x0001
    mov word [bp - 2], gdt

    lgdt [bp - 6]

    add sp, 6

    pop bp
    ret

cpu 8086
