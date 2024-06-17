bits 32
section .realmode

; Variables
align 4
s_stack: times 1024 db 0
s_registers:
    .eax: dd 0
    .ebx: dd 0
    .ecx: dd 0
    .edx: dd 0
    .ebp: dd 0
    .esi: dd 0
    .edi: dd 0
    .eflags: dd 0
    .ds: dd 0
    .es: dd 0
    .end:
s_interruptsEnabled: dd 0
s_savedEbp: dd 0
s_idtr:
    .limit dw 0x3ff
    .base dd 0x00000000
s_savedIdtr:
    .limit dw 0
    .base dd 0

global bioscall
bioscall:
%define p_vector ebp + 8
%define p_registers ebp + 12
    push ebp
    mov ebp, esp

    ; Save registers
    push ebx
    push edi
    push esi

    ; Copy register values
    mov esi, [p_registers]
    mov edi, s_registers
    mov ecx, s_registers.end - s_registers
    repz movsb

    ; Save IDTR
    sidt [s_savedIdtr]

    ; Generate the "int p_vector" instruction
    mov al, [p_vector]
    mov [.int_vector + 1], al

    ; Save the value of EBP
    mov [s_savedEbp], ebp

    ; Save the state of interrupts
    pushfd
    pop eax

    test eax, 1 << 9
    jz .interrupts_disabled

.interrupts_enabled:
    mov dword [s_interruptsEnabled], 1
    jmp .go_to_real_mode

.interrupts_disabled:
    mov dword [s_interruptsEnabled], 0

; Switch back to real mode
.go_to_real_mode:
    ; Disable interrupts
    cli

    ; Disable NMI
    in al, 0x70
    or al, 0x80
    out 0x70, al

    ; Jump to 16-bit protected mode
    jmp 0x0018:.pmode16

.pmode16:
bits 16
    ; Load 16-bit segments
    mov ax, 0x20
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Load real mode IDT
    lidt [s_idtr]

    ; Disable protected mode
    mov eax, cr0
    and al, 0xfe
    mov cr0, eax

    ; Jump to 16-bit real-mode
    jmp 0x0000:.real_mode

.real_mode:
    ; Initialize stack segment
    xor ax, ax
    mov ss, ax
    mov sp, s_stack + 1024

    mov eax, [ss:s_registers.eflags]
    push eax
    mov ax, [ss:s_registers.ds]
    mov ds, ax
    mov ax, [ss:s_registers.es]
    mov es, ax
    mov eax, [ss:s_registers.eax]
    mov ebx, [ss:s_registers.ebx]
    mov ecx, [ss:s_registers.ecx]
    mov edx, [ss:s_registers.edx]
    mov ebp, [ss:s_registers.ebp]
    mov esi, [ss:s_registers.esi]
    mov edi, [ss:s_registers.edi]
    popfd

.int_vector:
    int 0x00

    mov [s_registers.eax], eax
    mov [s_registers.ebx], ebx
    mov [s_registers.ecx], ecx
    mov [s_registers.edx], edx
    mov [s_registers.ebp], ebp
    mov [s_registers.esi], esi
    mov [s_registers.edi], edi
    pushfd
    pop eax
    mov [s_registers.eflags], eax
    mov ax, ds
    mov [s_registers.ds], ax
    mov ax, es
    mov [s_registers.es], ax

    ; Enable protected-mode
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; Jump to protected mode
    jmp dword 0x0008:.pmode

.pmode:
bits 32
    ; Restore segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Restore stack
    mov ebp, [s_savedEbp]
    mov esp, ebp
    sub esp, 12

    ; Reload IDT
    lidt [s_savedIdtr]

    ; Re-enable interrupts
    test dword [s_interruptsEnabled], 1
    jz .enable_nmi

    sti

.enable_nmi:
    ; Enable NMI
    in al, 0x70
    and al, 0x7f
    out 0x70, al

    ; Copy registers
    mov esi, s_registers
    mov edi, [p_registers]
    mov ecx, s_registers.end - s_registers
    repz movsb

.epilogue:
    pop esi
    pop edi
    pop ebx
    pop ebp
    ret

%undef p_vector
%undef p_registers
