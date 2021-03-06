bits 32

%define RM_CONTEXT_ADDRESS 0x00008000
%define RM_PROCEDURE_ADDRESS 0x00000500
%define ADDR_IN_BIOS_CALL_WRAPPED(symbol) (RM_PROCEDURE_ADDRESS + symbol - bios_call_wrapped)

extern kernel_pageDirectory

section .text

global bios_init
bios_init:
    push esi
    push edi
    push ebx

    ; Copy the bios_call_wrapped procedure
    mov esi, bios_call_wrapped
    mov edi, RM_PROCEDURE_ADDRESS
    mov ecx, bios_call_wrapped.end - bios_call_wrapped

    repz movsb

    ; Initialize the page directory
    mov eax, 0x0000008b
    mov ecx, 256
    mov ebx, bios_call.pageDirectory

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

global bios_call
bios_call:
    push ebp
    mov ebp, esp
    
    ; Save context
    pushfd
    push esi
    push edi
    push ebx
    push ebp

    ; Save ESP
    mov [.saved_esp], esp
    
    ; copy interrupt context
    mov esi, [ebp + 12]
    mov edi, RM_CONTEXT_ADDRESS
    mov ecx, 10
    repz movsd

    ; Modify int opcode
    mov eax, [ebp + 8]
    lea ebx, [ADDR_IN_BIOS_CALL_WRAPPED(bios_call_wrapped.int) + 1]
    mov [ebx], al

    cli
    
    ; Set identity-paged page directory
    mov eax, bios_call.pageDirectory - 0xc0000000
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

    ; Restore EBP
    pop ebp

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
    popfd
    pop ebp
    ret
    
section .bss
align 4096
.pageDirectory:
    resb 4096

.saved_esp:
    resd 1

section .text

bios_call_wrapped:
    ; Disable protected mode
    mov eax, cr0
    and eax, 0xfffffffe
    mov cr0, eax

    jmp dword 0x00:ADDR_IN_BIOS_CALL_WRAPPED(.next)

.next:
    bits 16

    mov ax, 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    sidt [ADDR_IN_BIOS_CALL_WRAPPED(.pmode_idtr)]
    lidt [ADDR_IN_BIOS_CALL_WRAPPED(.rmode_idtr)]

.loadInterruptContext:
    xor ax, ax
    mov ss, ax
    mov sp, RM_CONTEXT_ADDRESS

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
    
    lidt [ADDR_IN_BIOS_CALL_WRAPPED(.pmode_idtr)]

    mov eax, cr0
    or eax, 0x80010001
    mov cr0, eax

    jmp dword 0x08:bios_call.returnFromInt

.pmode_idtr:
    dw 0
    dd 0

.rmode_idtr:
    dw 0x3ff
    dd 0

.end:
