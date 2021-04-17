%define KERNEL_INITSTACK_SIZE 16384

extern __kernel_end
extern kmain

bits 32

section .bootstrap

global arch_bootstrap
arch_bootstrap:
    ; Compute kernel length in pages (lower memory included)
    mov eax, ((__kernel_end - 0xc0000000) + 0x00000fff)
    shr eax, 12

    ; Save in EDX for later
    mov edx, eax

    ; Compute the number of PSE entries
    mov ecx, eax
    shr ecx, 10

    ; Put the PSE entries in the page directory
    lea ebx, [kernel_pageDirectory]
    mov eax, 0x0000008b

    test ecx, ecx
    jz .fillPageDirectory_pse_end

.fillPageDirectory_pse:
    mov [ebx], eax ; Identity-mapped pages
    mov [ebx + 768 * 4], eax ; Higher-half pages
    add ebx, 4 ; Set EBX to the next entry of the page directory
    add eax, 0x00400000 ; We will map 4 MiB further
    loop .fillPageDirectory_pse ; Loop until we have < 4 MiB to map

.fillPageDirectory_pse_end:
    ; Initialize the page directory entry pointing to the page table
    mov ecx, eax
    lea eax, [kernel_pageTable]
    or eax, 0x0000000b
    mov [ebx], eax
    mov [ebx + 768 * 4], eax

    ; Clear page directory entry bits and set page table entry bits
    mov eax, ecx
    and eax, 0xfffff000
    or eax, 0x0000000b

    ; We are now filling the page table
    mov ebx, kernel_pageTable

    ; Get the number of page table entries to fill
    mov ecx, edx
    and ecx, 0x000003ff
    test ecx, ecx
    jz .fillPageTable_end

.fillPageTable:
    mov [ebx], eax ; Set page table entry
    add ebx, 4 ; Set EBX to the next entry of the page table
    add eax, 0x00001000 ; We are mapping 4 KiB further
    loop .fillPageTable ; Loop until we have mapped all the kernel

.fillPageTable_end:
    ; Set CR3 to the kernel page directory
    lea eax, [kernel_pageDirectory]
    mov cr3, eax

    ; Enable PSE
    mov eax, cr4
    or eax, 0x00000010
    mov cr4, eax

    ; Enable paging and write-protect
    mov eax, cr0
    or eax, 0x80010000
    mov cr0, eax

    ; Jump to higher-half code
    lea ecx, [.higherHalf]
    jmp ecx

section .text

.higherHalf:
    ; Setup stack
    mov esp, kernel_initStack_bottom

    ; Reload GDT
    lgdt [gdtr + 0xc0000000]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp dword 0x08:kmain

section .bootstrap

global gdt
gdt:
.nullEntry:
    times 8 db 0
.codeSegment32_r0:
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00
.dataSegment32_r0:
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00
.codeSegment32_r3:
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0xfa, 0xcb, 0x00
.dataSegment32_r3:
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0xf2, 0xcb, 0x00
.codeSegment16:
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0x0f, 0x00
.dataSegment16:
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0x0f, 0x00
.tss:
    times 8 db 0
.end:

gdtr:
    dw gdt.end - gdt - 1
    dd gdt

global kernel_pageDirectory
align 4096
kernel_pageDirectory:
    times 4096 db 0

global kernel_pageTable
align 4096
kernel_pageTable:
    times 4096 db 0

section .bss
global kernel_initStack_top
align 4
kernel_initStack_top:
    resb KERNEL_INITSTACK_SIZE
global kernel_initStack_bottom
kernel_initStack_bottom:
