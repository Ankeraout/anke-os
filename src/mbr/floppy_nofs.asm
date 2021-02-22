bits 16

%define KERNEL_SIZE 0x10000
%define KERNEL_SIZE_SECTORS (KERNEL_SIZE >> 9)
%define KERNEL_LOADADDR_LOW 0x10000
%define KERNEL_LOADADDR_HIGH 0x100000

org 0x7c00

_start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xfffe

enableA20:
    call checkA20
    test ax, ax
    jnz readKernel

.bios:
    mov ax, 0x2401
    int 0x15

    call checkA20
    test ax, ax
    jnz readKernel

.keyboardController:
    cli

    call .wait2
    mov al, 0xad
    out 0x64, al

    call .wait2
    mov al, 0xd0
    out 0x64, al

    call .wait1
    in al, 0x60
    push ax

    call .wait2
    mov al, 0xd1
    out 0x64, al

    call .wait2
    pop ax
    or al, 2
    out 0x60, al

    call .wait2
    mov al, 0xae
    out 0x64, al

    call .wait2
    sti

    jmp .checkAfterKeyboard

.wait1:
    in al, 0x64
    test al, 1
    jnz .wait1
    ret

.wait2:
    in al, 0x64
    test al, 1
    jz .wait2
    ret

.checkAfterKeyboard:
    call checkA20
    test ax, ax
    jnz readKernel

.fast:
    in al, 0x92
    or al, 2
    out 0x92, al

    mov cx, 100

.retryAfterFast:
    call checkA20
    test ax, ax
    jnz readKernel
    loop .retryAfterFast

.failed:
    jmp failure3

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
    cli

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

    mov esi, KERNEL_LOADADDR_LOW
    mov edi, KERNEL_LOADADDR_HIGH
    mov ecx, KERNEL_SIZE >> 2

    rep movsd

    jmp KERNEL_LOADADDR_HIGH

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

failure3:
    mov ax, 0xb800
    mov es, ax
    mov byte [es:0x0000], '3'
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

checkA20:
    push es
    mov ax, 0xff00
    mov es, ax
    mov byte [es:0x2000], 0x55
    mov byte [ds:0x1000], 0xaa
    mov al, [es:0x2000]
    cmp al, 0x55
    jz .enabled
    
.disabled:
    pop es
    mov ax, 0
    ret

.enabled:
    pop es
    mov ax, 1
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
    dw KERNEL_SIZE_SECTORS

lba_current_seg:
    dw KERNEL_LOADADDR_LOW >> 4

spt:
    dw 18

spt2:
    dw 36
    
times 510-($-$$) db 0
dw 0xaa55
