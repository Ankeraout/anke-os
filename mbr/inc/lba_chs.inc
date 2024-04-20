; Input:
; AX = LBA
;
; Output:
; CH = cylinder[0, 7]
; CL[0, 5] = sector
; CL[6, 7] = cylinder[8, 9]
; DH = head
lba_to_chs:
    xor dx, dx                          ; DX = 0
    div word [bpb.sectors_per_track]    ; tmp = lba / bpb.sectors_per_track
    mov cx, dx                          ; sector = lba % bpb.sectors_per_track
    inc cx                              ; sector += 1
    xor dx, dx                          ; DX = 0
    div word [bpb.heads]                ; cylinder = tmp / bpb.heads
                                        ; head = tmp % bpb.heads
    
    mov dh, dl                          ; DH = head
    mov ch, al                          ; CH = cylinder & 0xff
    shr ax, 1
    shr ax, 1
    and ax, 0xc0
    or cl, al                           ; CL = sector | (cylinder >> 2) & 0xc0
    
    ret
