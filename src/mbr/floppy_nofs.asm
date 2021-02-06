bits 16

org 0x7c00

_start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xfffe

readKernel:
    mov ah, 0x00
    int 0x13

    jc failure1

.loop:
    call lbaToChs

    push es
    mov ax, [lba_current_seg]
    mov es, ax
    mov ax, 0x0201
    xor bx, bx
    mov ch, [chs.cylinder]
    mov cl, [chs.sector]
    mov dh, [chs.head]
    mov dl, 0x00
    int 0x13
    pop es

    jc failure2

    add word [lba_current_seg], 32
    inc word [lba]
    dec word [remaining_lba]

    jnz .loop

loadGdt:
    lgdt [gdtr]

enterProtectedMode:
    mov eax, cr0
    inc al
    mov cr0, eax

    jmp dword 0x08:protectedMode

protectedMode:
    bits 32
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x0000fffe

    mov esi, 0x10000
    mov edi, 0x100000
    mov ecx, 0x10000 >> 2
    rep movsd

    jmp 0x100000

failure1:
    bits 16
    mov ax, 0xb800
    mov es, ax
    mov byte [es:0x0000], '1'
    cli
    hlt

failure2:
    mov ax, 0xb800
    mov es, ax
    mov byte [es:0x0000], '2'
    cli
    hlt

lbaToChs:
    mov ax, [lba]
    xor dx, dx
    div word [spt2]
    mov [chs.cylinder], ax

    mov ax, dx
    xor dx, dx
    div word [spt]
    mov [chs.head], al

    mov ax, [lba]
    xor dx, dx
    div word [spt]
    inc dx
    mov [chs.sector], dl

    ret
    
gdtr:
    dw (gdt_end - gdt) - 1
    dd gdt

gdt:
    db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00
gdt_end:

lba:
    dw 1

chs:
    .cylinder dw 0
    .head db 0
    .sector db 0

remaining_lba:
    dw 0x80

lba_current_seg:
    dw 0x1000

spt:
    dw 18

spt2:
    dw 36
    
times 510-($-$$) db 0
dw 0xaa55
