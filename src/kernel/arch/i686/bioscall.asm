bits 32

%define RM_CONTEXT_ADDRESS 0x00000f00
%define RM_PROCEDURE_ADDRESS 0x00000500
%define ADDR_IN_BIOSCALL_WRAPPED(symbol) (RM_PROCEDURE_ADDRESS + symbol - bioscall_wrapped)

extern kernel_pageDirectory

section .text

global bioscall_init
bioscall_init:
    push esi
    push edi
    push ebx

    ; Copy the bioscall_wrapped procedure to 0x00010000
    mov esi, bioscall_wrapped
    mov edi, RM_PROCEDURE_ADDRESS
    mov ecx, bioscall_wrapped.end - bioscall_wrapped

    repz movsb

    ; Initialize the page directory
    mov eax, 0x0000008b
    mov ecx, 256
    mov ebx, bioscall.pageDirectory

.loop:
    mov [ebx], eax
    mov [ebx + 4 * 768], eax
    add ebx, 4
    add eax, 0x00400000
    loop .loop

    pop ebx
    pop edi
    pop esi
    ret

global bioscall
bioscall:
    push ebp
    mov ebp, esp

    push esi
    push edi
    push ebx
    
    ; copy interrupt context
    mov esi, [ebp + 12]
    mov edi, RM_CONTEXT_ADDRESS
    mov ecx, 10
    repz movsd

    ; Modify int opcode
    mov eax, [ebp + 8]
    lea ebx, [ADDR_IN_BIOSCALL_WRAPPED(bioscall_wrapped.int) + 1]
    mov [ebx], al

    cli
    
    ; Set identity-paged page directory
    mov eax, bioscall.pageDirectory - 0xc0000000
    mov cr3, eax

    ; Jump to lower-half code
    jmp .lowerHalf - 0xc0000000

.lowerHalf:
    mov ax, 0x30
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Save ESP
    mov [.saved_esp], esp

    ; Disable paging
    mov eax, cr0
    and eax, 0x7ffeffff
    mov cr0, eax
    
    jmp dword 0x28:RM_PROCEDURE_ADDRESS

.returnFromInt:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, [.saved_esp]

    ; copy interrupt context
    mov edi, [ebp + 12]
    mov esi, RM_CONTEXT_ADDRESS
    mov ecx, 10
    repz movsd

    ; Set kernel cr3
    mov eax, kernel_pageDirectory
    mov cr3, eax

    pop ebx
    pop edi
    pop esi
    pop ebp
    ret
    
section .bss
.pageDirectory:
    resb 4096

.saved_esp:
    resd 1

section .text

bioscall_wrapped:
    ; Disable protected mode
    mov eax, cr0
    and eax, 0xfffffffe
    mov cr0, eax

    jmp dword 0x00:ADDR_IN_BIOSCALL_WRAPPED(.next)

.next:
    bits 16

    mov ax, 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0xfffe
    sidt [ADDR_IN_BIOSCALL_WRAPPED(.pmode_idtr)]
    lidt [ADDR_IN_BIOSCALL_WRAPPED(.rmode_idtr)]

.loadInterruptContext:
    mov ax, RM_CONTEXT_ADDRESS >> 4
    mov ss, ax
    mov sp, 0

    pop edi
    pop esi
    pop ebp
    pop ebx
    pop edx
    pop ecx
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popfd

.int:
    int 0

.saveInterruptContext:
    pushfd
    push ds
    push es
    push fs
    push gs
    push eax
    push ecx
    push edx
    push ebx
    push ebp
    push esi
    push edi
    
    lidt [ADDR_IN_BIOSCALL_WRAPPED(.pmode_idtr)]

    mov eax, cr0
    or eax, 0x80010001
    mov cr0, eax

    jmp dword 0x08:bioscall.returnFromInt

.pmode_idtr:
    dw 0
    dd 0

.rmode_idtr:
    dw 0x3ff
    dd 0

.end:
