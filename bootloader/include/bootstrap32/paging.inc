paging_init:
    ; PML4 at 0x1000
    ; PDPT at 0x2000
    ; PD at 0x3000
    ; PT at 0x4000

    ; Zero out paging structures
    push edi
    mov edi, 0x1000
    mov ecx, 0x1000
    xor eax, eax
    repz stosd
    pop edi

    ; Initialize PML4 entry
    mov dword [0x1000], 0x00002003
    
    ; Initialize PDPT entry
    mov dword [0x2000], 0x00003003

    ; Initialize PD entry
    mov dword [0x3000], 0x00004003

    ; Initialize PT entries
    mov edx, 0x4000
    mov eax, 0x00000003
    mov ecx, 512

    .loop:
        mov [edx], eax
        add edx, 8
        add eax, 0x00001000
        loop .loop
    
    ret
