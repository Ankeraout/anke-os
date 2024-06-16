; Input:
; AX = LBA
; ES:BX = buffer
; CX = number of sectors to read (must be > 0)
read_sectors:
    push ax
    push cx
    
    call lba_to_chs
    
    mov dl, [bpb.drive_number]
    mov ax, 0x0201
    int 0x13
    
    jc error_io

    mov cx, [bpb.bytes_per_sector]
    shr cx, 1
    shr cx, 1
    shr cx, 1
    shr cx, 1
    mov ax, es
    add ax, cx
    mov es, ax

    pop cx
    pop ax
    inc ax

    loop read_sectors

    ret
