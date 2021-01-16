%define KERNEL_STACK_SIZE 16384

bits 32

extern __kernel_end
extern kernel_main

section .bootstrap

global arch_bootstrap
arch_bootstrap:
    ; Compute kernel length in pages
    mov eax, ((__kernel_end - 0xc0000000) + 0x00000fff)
    shr eax, 12

    mov edx, eax ; Save in EDX for later

    ; Compute the number of PSE entries
    mov ecx, eax
    shr ecx, 10

    ; Put the PSE entries
    lea ebx, [kernel_pageDirectory]
    mov eax, 0x0000008b

    test ecx, ecx
    jz .fillPageDirectoryPSE_end

.fillPageDirectoryPSE:
    mov [ebx], eax ; Identity mapping
    mov [ebx + 768 * 4], eax ; Higher-half mapping
    add ebx, 4 ; Set EBX to next page directory entry
    add eax, 0x00400000 ; Increase the value of the entry by 4 MiB
    loop .fillPageDirectoryPSE ; Loop until we have less than 4 MiB to map

.fillPageDirectoryPSE_end:
    ; Initialize the value of the page directory entry pointing to the page
    ; table
    mov ecx, eax
    lea eax, [pageTable]
    or eax, 0x0000000b
    mov [ebx], eax
    mov [ebx + 768 * 4], eax

    mov eax, ecx
    and eax, 0xfffff000
    or eax, 0x0000000b

    mov ebx, pageTable
    
    mov ecx, edx
    and ecx, 0x000003ff
    test ecx, ecx
    jz .fillPageTable_end

.fillPageTable:
    mov [ebx], eax
    add ebx, 4
    add eax, 0x00001000
    loop .fillPageTable

.fillPageTable_end:
    ; Set CR3
    lea eax, [kernel_pageDirectory]
    mov cr3, eax

    ; Enable paging and PSE
    mov eax, cr4
    or eax, 0x00000010
    mov cr4, eax

    mov eax, cr0
    or eax, 0x80010000
    mov cr0, eax

    ; Jump to higher-half code
    lea ecx, [.higherHalf]
    jmp ecx

section .text

.higherHalf:
    ; Setup stack
    mov esp, kernel_stack_bottom

    ; Reload GDT
    lgdt [gdtr + 0xc0000000]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp dword 0x08:.higherHalf_reloadGDT

.higherHalf_reloadGDT:
    ; Jump to kernel C code
    jmp kernel_main

section .bootstrap

align 16
gdt:
.nullEntry:
    times 8 db 0
.codeSegment32_r0:
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00
.dataSegment32_r0:
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00
.codeSegment32_r3:
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0xfa, 0xcf, 0x00
.dataSegment32_r3:
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0xf2, 0xcf, 0x00
.codeSegment16:
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0x0f, 0x00
.dataSegment16:
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0x0f, 0x00
.tss:
    times 8 db 0
.end:
    
align 16
gdtr:
    dw gdt.end - gdt - 1
    dd gdt

align 4096
global kernel_pageDirectory
kernel_pageDirectory:
    times 4096 db 0

pageTable:
    times 4096 db 0

section .bss
kernel_stack_top:
    resb KERNEL_STACK_SIZE
global kernel_stack_bottom
kernel_stack_bottom:
