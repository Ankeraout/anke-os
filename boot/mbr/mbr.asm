bits 16
cpu 8086

%define C_PARTITION_TABLE_ENTRY_SIZE 16

_start:
    ; Prepare stack
    mov ax, 0x8000
    mov ss, ax
    xor sp, sp

    ; Relocate MBR to 07e0:0000
    mov ax, 0x7c0
    mov ds, ax
    mov ax, 0x7e0
    mov es, ax
    xor si, si
    xor di, di
    mov cx, 512
    repz movsb

    jmp 0x7e0:afterRelocation

afterRelocation:
    ; DS = 0x7e0
    ; ES = 0x7c0
    push es
    push ds
    pop es
    pop ds

    ; Save boot drive number
    mov [bootDrive], dl

    ; Prepare for checking partitions
    mov si, partitionTable

checkNextPartition:
    ; If all partitions were checked, display an error message.
    cmp si, bootSignature
    jz noBootablePartition

    ; Check if the partition is enabled
    test byte [si], 0x80
    jnz enabledPartition

    add si, C_PARTITION_TABLE_ENTRY_SIZE
    jmp checkNextPartition

enabledPartition:
    ; Load the VBR
    mov ax, 0x0201
    xor bx, bx
    mov cx, [si + 2]
    mov dh, [si + 3]
    mov dl, [bootDrive]
    int 0x13
    jc failedToReadMbr

    ; Check boot signature
    cmp word [es:bootSignature], 0xaa55
    jnz partitionNotBootable

    ; Prepare segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, ax

    ; Jump to the VBR
    jmp 0x0000:0x7c00

partitionNotBootable:
    mov si, strPartitionNotBootable
    jmp printError

failedToReadMbr:
    mov si, strFailedToReadMbr
    jmp printError

noBootablePartition:
    mov si, strNoBootablePartition
    jmp printError

printError:
    call puts
    jmp rebootOnKeyPress

puts:
    lodsb
    test al, al
    jz .end
    mov ah, 0x0e
    int 0x10
    jmp puts
    
    .end:
        ret

rebootOnKeyPress:
    xor ax, ax
    int 0x16
    int 0x19

; Variables
bootDrive: db 0
currentPartitionOffset: dw 0x1be
strPartitionNotBootable: db "Error: Invalid MBR signature.", 13, 10, 0
strFailedToReadMbr: db "Error: Failed to read MBR.", 13, 10, 0
strNoBootablePartition: db "Error: No bootable partition.", 13, 10, 0

times 0x1be-($-$$) db 0xff

partitionTable:
    times 64 db 0

bootSignature:
    dw 0xaa55
