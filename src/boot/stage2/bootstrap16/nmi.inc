nmi_disable:
    in al, 0x70
    or al, 0x80
    out 0x70, al
    ret
