bits 16
org 0x7c00
cpu 8086

%define C_MAX_FILE_SIZE 524288

_start:
    jmp main

times 0x03-($-$$) nop

%include "bpb_fat.inc"
%include "bpb_fat12_16.inc"

main:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    xor sp, sp

    jmp 0x0000:next

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
    add ax, [bpb.reserved_sectors]
    mov si, ax

    ; Compute the size of the root directory in sectors
    mov ax, 32
    mul word [bpb.root_directory_entries]
    div word [bpb.bytes_per_sector]
    mov cx, ax

    ; Compute the LBA of cluster 2 (the first cluster)
    add ax, si
    mov [first_cluster_sector], ax

    ; Read the root directory in memory
    mov ax, si
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
    or si, 0x001f
    inc si
    loop search_file_loop

file_not_found:
    jmp error_io

file_found:
    ; Check file size
    cmp word [si + 19], C_MAX_FILE_SIZE >> 16
    ja error_io
    jnae .size_ok

    cmp word [si + 17], C_MAX_FILE_SIZE & 0xffff
    jnz error_io

.size_ok:
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
    mov ax, 0x0800
    mov es, ax
    xor bx, bx
    
    read_file_loop:
        call read_cluster

        mov ax, [current_cluster]
        cmp ax, 0x0ff8
        jnae read_file_loop

execute_file:
    ; Set DL to the BIOS boot drive
    mov dl, [bpb.drive_number]

    ; Jump to the loaded file.
    jmp 0x0000:0x8000

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

%include "lba_chs.inc"
%include "puts.inc"
%include "read_sectors.inc"

file_name           db "BOOT    BIN"
msg_error_io        db "Failed to load /BOOT.BIN.", 13, 10, 0

; Add boot signature at the end
times 0x1fe-($-$$) db 0
dw 0xaa55

; Variables start here
section .bss
first_cluster_sector resw 1
current_cluster resw 1

; Align
times 0x20-($-$$) resb 1

fat_buffer resb 6144 ; FAT12's FAT is max. 12 sectors (= 6 KiB) long.
root_directory_buffer:
