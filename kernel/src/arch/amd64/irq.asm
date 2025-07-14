bits 64

section .text

global irq_endOfInterrupt
irq_endOfInterrupt:
    mov al, 0x20
    cmp rdi, 8
    jb .master

    .slave:
        out 0xa0, al

    .master:
        out 0x20, al

    .end:
        ret
