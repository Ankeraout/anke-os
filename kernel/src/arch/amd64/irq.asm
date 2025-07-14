bits 64

section .text

global irq_endOfInterrupt
irq_endOfInterrupt:
    mov al, 0x20
    out 0x20, al
    ret
