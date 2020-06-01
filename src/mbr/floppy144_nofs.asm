bits 16
org 0x0000

_start:
    ; Initialize registers
    mov ax, 0x7c0
    mov ds, ax
    mov es, ax
    mov ax, 0x9000
    mov ss, ax
    mov sp, 0xfffe

    ; Save the boot drive number
    mov [bootDrive], dl

    ; Initialize boot drive
    xor ah, ah
    int 0x13
    jc halt

.loadSecondStage:
    ; Read the 64 first sectors into memory at address 0x00010000
    mov ax, 0x1000
    mov es, ax
    mov ah, 0x02
    mov al, 64
    xor bx, bx
    mov cx, 0x0002
    mov dh, 0
    int 0x13
    jc halt

.loadSecondStage_part2:
    or bh, 0x80
    mov cx, 0x0303
    mov dh, 1
    int 0x13
    jc halt

.launchSecondStage:
    ; Jump to the loaded executable
    jmp 0x1000:0x0000

halt:
    cli
    hlt
    jmp halt

bootDrive db 0

times 510-($-$$) db 0
dw 0xaa55
