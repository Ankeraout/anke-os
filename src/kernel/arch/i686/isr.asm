section .text
global isr_handler0_7
isr_handler0_7:
    mov al, 0x20
    out 0x20, al
    iret

global isr_handler8_15
isr_handler8_15:
    mov al, 0x20
    out 0xa0, al
    out 0x20, al
    iret
