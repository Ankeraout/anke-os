bits 16
org 0x0000
cpu 8086

_start:
    jmp main
    nop

bpb.oem_identifier          times 8 db 0x00
bpb.bytes_per_sector        dw 0
bpb.sectors_per_cluster     db 0
bpb.reserved_sectors        dw 0
bpb.fat_count               db 0
bpb.root_directory_entries  dw 0
bpb.total_sectors_16        dw 0
bpb.media_descriptor_type   db 0
bpb.sectors_per_fat         dw 0
bpb.sectors_per_track       dw 0
bpb.heads                   dw 0
bpb.hidden_sectors          dd 0
bpb.total_sectors_32        dd 0
bpb.drive_number            db 0
bpb.flags                   db 0
bpb.signature               db 0
bpb.volume_id               times 4 db 0
bpb.volume_label            times 11 db 0
bpb.system_identifier       times 8 db 0

main:
    mov ax, 0x7c0
    mov ds, ax
    mov es, ax

    mov ax, 0x9000
    mov ss, ax
    mov sp, 0xf000

    jmp 0x7c0:next

next:
    ; Save drive number
    mov [bpb.drive_number], dl

    ; Initialize drive
    xor ah, ah
    int 0x13
    jc error_io

    ; Compute first root directory LBA
    mov al, [bpb.fat_count]
    xor ah, ah
    mul word [bpb.sectors_per_fat]
    mov [fat_size_sectors], ax
    add ax, [bpb.reserved_sectors]
    mov [first_root_sector], ax

    ; Compute the size of the root directory in sectors
    mov ax, [bpb.root_directory_entries]
    shl ax, 1
    shl ax, 1
    shl ax, 1
    shl ax, 1
    shl ax, 1
    xor dx, dx
    div word [bpb.bytes_per_sector]
    mov [root_directory_size_sectors], ax

    ; Compute the LBA of cluster 2 (the first cluster)
    add ax, [first_root_sector]
    mov [first_cluster_sector], ax

    ; Read the root directory in memory
    mov ax, [first_root_sector]
    mov cx, [root_directory_size_sectors]
    mov bx, root_directory_buffer

    push es
    call read_sectors
    pop es

    ; Search for the entry of the BOOT.BIN file
    mov cx, [bpb.root_directory_entries]
    mov si, root_directory_buffer

search_file_loop:
    mov di, file_name
    push cx
    mov cx, 11
    repz cmpsb
    pop cx
    jz file_found
    and si, 0xffe0
    add si, 32
    loop search_file_loop

file_not_found:
    jmp error_io

file_found:
    ; Check file size
    mov ax, [si + 19]
    test ax, ax
    jnz error_io

    mov ax, [si + 17]
    test ax, ax
    jz error_io

    cmp ax, 32768
    ja error_io

    ; Save file cluster
    mov ax, [si + 15]
    mov [current_cluster], ax

    ; Load FAT into memory
    mov ax, [bpb.reserved_sectors]
    mov bx, fat_buffer
    mov cx, [bpb.sectors_per_fat]

    push es
    call read_sectors
    pop es

read_file:
    ; Read BOOT.BIN
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    
    read_file_loop:
        call read_cluster

        mov ax, [current_cluster]
        cmp ax, 0x0ff8
        jnae read_file_loop

execute_file:
    ; Jump to the loaded file.
    jmp 0x1000:0x0000

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

; Input:
; - [current_cluster]: current cluster number
;
; Output:
; - [current_cluster]: new cluster number
read_cluster_entry:
    mov si, [current_cluster]
    shr si, 1
    add si, [current_cluster]
    mov ax, [fat_buffer + si]
    mov cx, [current_cluster]
    test cx, 1
    jz read_cluster_entry_even

    read_cluster_entry_odd:
        shr ax, 1
        shr ax, 1
        shr ax, 1
        shr ax, 1
        jmp read_cluster_entry_epilogue

    read_cluster_entry_even:
        and ax, 0x0fff

    read_cluster_entry_epilogue:
        mov [current_cluster], ax
        ret

; Input:
; - [current_cluster]: current cluster number
; - BX: buffer
;
; Output:
; - [current_cluster]: new cluster number
read_cluster:
    mov al, [bpb.sectors_per_cluster]
    xor ah, ah
    mov cx, ax
    mov dx, [current_cluster]
    dec dx
    dec dx
    mul dx

    add ax, [first_cluster_sector]

    call read_sectors
    call read_cluster_entry

    ret

error_io:
    mov si, msg_error_io
    call puts
    jmp $

; Input:
; SI = message
puts:
    lodsb
    test al, al
    jz puts_ret

    mov ah, 0x0e
    xor bx, bx
    int 0x10

    jmp puts

    puts_ret:
        ret

file_name           db "BOOT    BIN"
msg_error_io        db "Failed to load /BOOT.BIN.", 13, 10, 0

; Add boot signature at the end
times 0x1fe-($-$$) db 0
dw 0xaa55

; Variables start here
section .bss
fat_size_sectors resw 1
root_directory_size_sectors resw 1
first_root_sector resw 1
first_cluster_sector resw 1
current_cluster resw 1

; Align
times 0x20-($-$$) resb 1

fat_buffer resb 6144 ; FAT12's FAT is max. 12 sectors (= 6 KiB) long.
root_directory_buffer:
