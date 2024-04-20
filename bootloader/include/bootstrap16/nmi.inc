; void nmi_disable(void)
;
; Summary:
; Disables non-maskable interrupts (NMIs) by writing a value with the 7th bit
; set to I/O port 0x70.
nmi_disable:
    in al, 0x70
    or al, 0x80
    out 0x70, al
    ret
