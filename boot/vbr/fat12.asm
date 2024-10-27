bits 16
cpu 8086

_start:
    jmp main
    nop

%include "boot/vbr/fat/header_fat1216.inc"

main:
    ; Prepare segment registers
    mov ax, 0x7c0
    mov ds, ax
    mov es, ax
    mov ss, ax
    xor sp, sp

    ; Set CS to 0x7c0 and jump to the next instructions
    jmp 0x7c0:saveDriveNumber

saveDriveNumber:
    ; Save drive number for later
    mov [bpb.driveNumber], dl

    push es

    ; Get drive parameters
    mov ah, 0x08
    int 0x13

    pop es

    jc ioError

    ; CL = maximum sector number (bits 5-0)
    ; DH = maximum head number
    and cx, 0x003f
    mov [bpb.sectorsPerTrack], cx

    xor dh, dh
    inc dx
    mov [bpb.heads], dx

    ; Compute FAT LBA
    mov ax, [bpb.hiddenSectors]
    mov dx, [bpb.hiddenSectors + 2]
    add ax, [bpb.reservedSectors]
    adc dx, 0
    mov [fatLba], ax
    mov [fatLba + 2], dx

    ; Compute root directory LBA
    xor ah, ah
    mov al, [bpb.fatCount]
    mul word [bpb.sectorsPerFat]
    add ax, [fatLba]
    adc dx, [fatLba + 2]
    mov [rootDirectoryLba], ax
    mov [rootDirectoryLba + 2], dx

    ; Compute first cluster LBA
    mov cx, [bpb.rootDirectoryEntries]
    shr cx, 1
    shr cx, 1
    shr cx, 1
    shr cx, 1
    add ax, cx
    adc dx, 0
    mov [firstClusterLba], ax
    mov [firstClusterLba + 2], dx
    mov [rootDirectorySectors], cx

    ; Load FAT in memory
    mov si, fatLba
    mov di, lba
    mov cx, 2
    repz movsw

    mov bx, fat
    mov cx, [bpb.sectorsPerFat]
    
    call readSectors

    ; Find the file entry in the root directory
findFileEntry:
    mov cx, [rootDirectorySectors]
    mov ax, [rootDirectoryLba]
    mov dx, [rootDirectoryLba + 2]
    mov [lba], ax
    mov [lba + 2], dx

    .rootDirectoryLoop:
        push cx
        
        ; Read root directory sector
        mov bx, rootDirectoryBuffer
        call readSector

        mov ax, [bpb.bytesPerSector]
        mov cx, 32
        xor dx, dx
        div cx
        mov cx, ax

        mov si, rootDirectoryBuffer
        
    .sectorLoop:
        mov di, strFileName
        mov cx, 11
        repz cmpsb
        jz fileFound

        and si, 0xffe0
        add si, 32
        
        loop .sectorLoop
    
    .sectorLoopEnd:
        inc word [lba]
        adc word [lba + 2], 0
        pop cx
        loop .rootDirectoryLoop

    .rootDirectoryLoopEnd:
        jmp ioError

fileFound:
    ; Read file
    mov ax, 0x1000
    mov es, ax
    xor bx, bx

    and si, 0xffe0
    mov ax, [si + 26]

    ; Read until the end of the cluster chain
    .loop:
        mov [cluster], ax
        cmp ax, 0xff8
        jae .end
        
        ; Read cluster
        sub ax, 2
        mov cl, [bpb.sectorsPerCluster]
        xor ch, ch
        mul cx
        add ax, [firstClusterLba]
        adc dx, [firstClusterLba + 2]
        mov [lba], ax
        mov [lba + 2], dx
        call readSectors

        mov al, [bpb.sectorsPerCluster]
        xor ah, ah
        mul word [bpb.bytesPerSector]
        xor dx, dx
        mov cx, 16
        div cx
        mov dx, es
        add dx, ax
        mov es, dx

        ; Get next cluster
        mov si, [cluster]
        mov dx, si
        shr dx, 1
        add si, dx
        mov ax, [fat + si]

        test byte [cluster], 1
        jz .even

    .odd:
        xor dx, dx
        div cx

    .even:
        and ax, 0xfff
        jmp .loop

    .end:
        mov dl, [bpb.driveNumber]
        jmp 0x1000:0x0000

; Summary:
; Reads sectors.
;
; Input:
; - [lba]: The LBA number.
; - es:bx: Buffer
; - cx: Sector count
;
; Output:
; - [es:bx]: The read sector data.
readSectors:
    push es

    ; Read sectors
    .loop:
        push cx
        call readSector
        
        ; Increment LBA
        inc word [lba]
        adc word [lba + 2], 0

        ; Compute ES increment: bytes_per_sector / 16
        mov ax, [bpb.bytesPerSector]
        xor dx, dx
        mov cx, 16
        div cx

        ; Increment ES
        mov dx, es
        add dx, ax
        mov es, dx

        pop cx

        ; Loop
        loop .loop
    
    pop es
    ret

; Summary:
; Reads a sector.
;
; Input:
; - [lba]: The LBA number.
; - es:bx: Buffer
;
; Output:
; - [es:bx]: The read sector data.
readSector:
    mov ax, [lba]
    mov dx, [lba + 2]
    div word [bpb.sectorsPerTrack]
    inc dl
    mov cl, dl
    xor dx, dx
    div word [bpb.heads]
    mov dh, dl
    mov ch, al
    ror ah, 1
    ror ah, 1
    and ah, 0xc0
    or cl, ah
    mov ax, 0x0201
    mov dl, [bpb.driveNumber]
    int 0x13
    jc ioError
    ret

ioError:
    mov si, strIoError
    
    .loop:
        lodsb
        or al, al
        jz .end
        mov ah, 0x0e
        int 0x10
        jmp .loop

    .end:
        xor ax, ax
        int 0x16
        int 0x19

strFileName: db "BOOT    BIN"
strIoError: db "#I/O", 0

    ; Write 0s until boot signature
    times 0x1fe-($-$$) db 0
    
bootSignature:
    dw 0xaa55

section .bss
fat: resb 6144
rootDirectoryBuffer: resb 4096
fatLba: resd 1
rootDirectoryLba: resd 1
firstClusterLba: resd 1
lba: resd 1
cluster: resw 1
rootDirectorySectors: resw 1
