extern main

%define C_VBE_MODE_WIDTH 640
%define C_VBE_MODE_HEIGHT 480
%define C_VBE_MODE_BPP 32
%define C_MMAP_MAX_ENTRIES 128

bits 16
cpu 8086

section .bootstrap16

global _start
_start:
    mov ax, 0x1000
    mov ds, ax
    mov es, ax
    mov ax, 0x8f00
    mov ss, ax
    xor sp, sp

    ; Save BIOS boot drive number
    mov [infoStructure.bootDrive], dl
    
    ; Print version
    mov ax, strVersion
    push ax
    call puts

    ; Install #UD handler
    push es
    xor ax, ax
    mov es, ax
    mov word [es:0x0018], unsupportedCpu
    mov word [es:0x001a], 0x1000
    pop es

    ; Check if the CPU is a 64-bit capable CPU.
    cpu x64
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jnz supportedCpu

    cpu 8086
unsupportedCpu:
    ; Unsupported CPU: print an error message then halt
    mov ax, strUnsupportedCpu
    push ax
    call puts

halt:
    cli
    hlt
    jmp halt

puts:
    push bp
    mov bp, sp

    push si

    mov si, [bp + 4]
    
    .loop:
        lodsb
        test al, al
        jz .end

        mov ah, 0x0e
        int 0x10
        jmp .loop

    .end:
        pop si
        pop bp
        ret

cpu x64

supportedCpu:
    ; The bootloader is running on a 64-bit CPU in real mode.
    call checkPciSupport
    call getMemoryMap
    call getFramebuffer
    call enableA20

    ; Disable interrupts
    cli

    ; Load GDT
    lgdt [gdtr]

    ; Enable protected mode
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; Reload segment registers
    mov ax, 0x0010
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Initialize stack
    mov esp, 0x9f000
    jmp dword 0x0008:protectedMode + 0x10000

protectedMode:
    bits 32
    ; Pass the info structure parameter
    push dword infoStructure + 0x10000

    ; Jump to C code
    mov ecx, main
    call ecx

bits 16

checkPciSupport:
    ; Check PCI support
    mov byte [infoStructure.pciSupported], 0

    mov ax, 0xb101
    xor edx, edx
    int 0x1a

    cmp edx, 0x20494350
    jne .return

    test ah, ah
    jnz .return

    test al, 1
    jz .return

    mov byte [infoStructure.pciSupported], 1

    .return:
        ret

getMemoryMap:
    ; Check if int 0x15, ax=0xe820 is supported
    mov di, memoryMap
    xor ebx, ebx

    .loop:
        mov eax, 0x0000e820
        mov ecx, 24
        mov edx, 0x534d4150
        int 0x15

        jc .error

        cmp eax, 0x534d4150
        jnz .error

        test ebx, ebx
        jz .end

        add di, 24
        add word [infoStructure.memoryMapSize], 24

        cmp word [infoStructure.memoryMapSize], 4096 - 24
        ja .error

        jmp .loop

    .end:
        ret

    .error:
        push word strErrorMemoryDetection
        call puts
        jmp halt

getFramebuffer:
    push bp
    mov bp, sp

    push fs
    push si

    call vbeGetControllerInfo

    ; Iterate through the mode table
    mov ax, [vbeControllerInfo + 16]
    mov fs, ax
    mov si, [vbeControllerInfo + 14]

    .loop:
        ; Check mode number
        mov ax, [fs:si]
        cmp ax, 0xffff
        jz .error

        ; Get mode info
        push ax
        add si, 2
        call vbeGetModeInfo
        add sp, 2

        ; Check mode
        test byte [vbeModeInfo], 0x80
        jz .loop

        cmp word [vbeModeInfo + 18], C_VBE_MODE_WIDTH
        jnz .loop

        cmp word [vbeModeInfo + 20], C_VBE_MODE_HEIGHT
        jnz .loop

        cmp byte [vbeModeInfo + 25], C_VBE_MODE_BPP
        jnz .loop

        ; Mode found
        mov eax, [vbeModeInfo + 40]
        mov [infoStructure.framebufferAddress], eax
        movzx eax, word [vbeModeInfo + 18]
        mov [infoStructure.framebufferWidth], eax
        movzx eax, word [vbeModeInfo + 20]
        mov [infoStructure.framebufferHeight], eax
        movzx eax, word [vbeModeInfo + 16]
        mov [infoStructure.framebufferPitch], eax
        movzx eax, byte [vbeModeInfo + 25]
        mov [infoStructure.framebufferBpp], eax
        movzx eax, byte [vbeModeInfo + 31]
        mov [infoStructure.framebufferRedBits], eax
        movzx eax, byte [vbeModeInfo + 32]
        mov [infoStructure.framebufferRedShift], eax
        movzx eax, byte [vbeModeInfo + 33]
        mov [infoStructure.framebufferGreenBits], eax
        movzx eax, byte [vbeModeInfo + 34]
        mov [infoStructure.framebufferGreenShift], eax
        movzx eax, byte [vbeModeInfo + 35]
        mov [infoStructure.framebufferBlueBits], eax
        movzx eax, byte [vbeModeInfo + 36]
        mov [infoStructure.framebufferBlueShift], eax

        ; Set video mode
        mov ax, 0x4f02
        mov bx, [fs:si - 2]
        or bx, 0x4000
        int 0x10

        cmp ax, 0x004f
        jnz .error

%undef l_modeIndex
        pop si
        pop fs
        pop bp
        ret

    .error:
        push word strErrorVbe
        call puts
        jmp halt

vbeGetControllerInfo:
    push bp
    mov bp, sp

    push di

    ; Get controller info
    mov ax, 0x4f00
    mov di, vbeControllerInfo
    int 0x10

    ; Check return values
    cmp ax, 0x004f
    jnz .error

    pop di
    pop bp
    ret

    .error:
        push word strErrorVbe
        call puts
        jmp halt

vbeGetModeInfo:
    push bp
    mov bp, sp
    push di

%define p_mode (bp + 4)

    mov ax, 0x4f01
    mov cx, [p_mode]
    mov di, vbeModeInfo
    int 0x10

    ; Compare return code
    cmp ax, 0x004f
    jnz .error

%undef p_mode

    pop di
    pop bp
    ret

    .error:
        push word strErrorVbe
        call puts
        jmp halt

enableA20:
    mov ax, 0x2401
    int 0x15
    ret

strUnsupportedCpu: db "Error: Your CPU is not supported. AnkeOS requires a 64-bit CPU.", 13, 10, 0
strVersion: db "AnkeOS bootloader 0.1.0", 13, 10, 0
strErrorMemoryDetection: db "Error: The memory map could not be retrieved from BIOS.", 13, 10, 0
strErrorVbe: db "Error: Failed to prepare framebuffer.", 13, 10, 0

align 8
gdt:
    .null: db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    .code32: db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00
    .data32: db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00
    .code64: db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xaf, 0x00
    .data64: db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xaf, 0x00
    .end:

gdtr:
    .size: dw gdt.end - gdt - 1
    .base: dd gdt + 0x10000

align 2
vbeControllerInfo: times 512 db 0
vbeModeInfo: times 256 db 0

align 8
infoStructure:
    .memoryMapAddress: dd 0x10000 + memoryMap
    .memoryMapSize: dd 0
    .framebufferAddress: dd 0
    .framebufferWidth: dd 0
    .framebufferHeight: dd 0
    .framebufferPitch: dd 0
    .framebufferBpp: dd 0
    .framebufferRedBits: dd 0
    .framebufferRedShift: dd 0
    .framebufferGreenBits: dd 0
    .framebufferGreenShift: dd 0
    .framebufferBlueBits: dd 0
    .framebufferBlueShift: dd 0
    .bootDrive: db 0
    .pciSupported: db 0

memoryMap: times C_MMAP_MAX_ENTRIES * 24 db 0
