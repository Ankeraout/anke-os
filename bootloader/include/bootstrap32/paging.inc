paging_init:
    ; PML4 at 0xc000
    ; PDPT at 0xd000
    ; PD at 0xe000
    ; PT at 0xf000

    ; Zero out paging structures
    push edi
    mov edi, 0xc000
    mov ecx, 0x1000
    xor eax, eax
    repz stosd
    pop edi

    ; Initialize PML4 entry
    mov dword [0xc000], 0x0000d003
    
    ; Initialize PDPT entry
    mov dword [0xd000], 0x0000e003

    ; Initialize PD entry
    mov dword [0xe000], 0x0000f003

    ; Initialize PT entries
    mov edx, 0xf000
    mov eax, 0x00000003
    mov ecx, 512

    .loop:
        mov [edx], eax
        add edx, 8
        add eax, 0x00001000
        loop .loop
    
    ret
