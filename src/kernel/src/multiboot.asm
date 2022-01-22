%define C_MULTIBOOT2_HEADER_MAGIC 0xe85250d6
%define C_MULTIBOOT2_HEADER_ARCHITECTURE 0x00000000
%define C_MULTIBOOT2_HEADER_LENGTH (multibootHeaderEnd - multibootHeaderStart)
%define C_MULTIBOOT2_HEADER_CHECKSUM \
    -( \
        C_MULTIBOOT2_HEADER_MAGIC \
        + C_MULTIBOOT2_HEADER_ARCHITECTURE \
        + C_MULTIBOOT2_HEADER_LENGTH \
    )

%define C_MULTIBOOT2_TAG_MANDATORY 0
%define C_MULTIBOOT2_TAG_OPTIONAL 1

%define C_MULTIBOOT2_TAG_ID_END 0
%define C_MULTIBOOT2_TAG_ID_INFORMATION_REQUEST 1
%define C_MULTIBOOT2_TAG_ID_FLAGS 4
%define C_MULTIBOOT2_TAG_ID_FRAMEBUFFER 5

%define C_MULTIBOOT2_INFO_ID_MEMORY_MAP 6
%define C_MULTIBOOT2_INFO_ID_FRAMEBUFFER 8

extern g_kernelStart
extern g_kernelEnd
extern kernelMain

bits 32

section .multiboot
align 8
multibootHeaderStart:
    ; Header
    dd C_MULTIBOOT2_HEADER_MAGIC
    dd C_MULTIBOOT2_HEADER_ARCHITECTURE
    dd C_MULTIBOOT2_HEADER_LENGTH
    dd C_MULTIBOOT2_HEADER_CHECKSUM

    ; Tag 1: Information request
    align 8
    dw C_MULTIBOOT2_TAG_ID_INFORMATION_REQUEST
    dw C_MULTIBOOT2_TAG_MANDATORY
    dd 16 ; Size
    dd C_MULTIBOOT2_INFO_ID_MEMORY_MAP
    dd C_MULTIBOOT2_INFO_ID_FRAMEBUFFER

    ; Tag 4: Flags tag
    align 8
    dw C_MULTIBOOT2_TAG_ID_FLAGS
    dw C_MULTIBOOT2_TAG_MANDATORY
    dd 12 ; Size
    dd 0x0001   ; Bit 0 set = at least one console should be available.
                ; Bit 1 cleared = kernel does not have EGA console mode support.

    ; Tag 5: Framebuffer tag
    align 8
    dw C_MULTIBOOT2_TAG_ID_FRAMEBUFFER
    dw C_MULTIBOOT2_TAG_MANDATORY
    dd 20 ; Size
    dd 0 ; Width: ? pixels
    dd 0 ; Height: ? pixels
    dd 32 ; Depth: 32 bits per pixel

    ; Tag 0: End of tags
    align 8
    dw C_MULTIBOOT2_TAG_ID_END
    dw C_MULTIBOOT2_TAG_MANDATORY
    dd 0x0008 ; Size

multibootHeaderEnd:

section .text_low
global _start
_start:
    ; Initialize lower-half kernel stack
    mov esp, kernelInitialStackBottom

    ; Initialize a basic flat GDT
    call kernelLoadInitialGdt

    ; Initialize paging structures
    call kernelInitPagingStructures

    ; Enable paging
    call enablePaging

    ; Jump to higher-half
    jmp kernelMain

kernelLoadInitialGdt:
    ; Initialize stack frame
    push ebp
    mov ebp, esp

    sub esp, 6 ; Allocate 6 bytes on the stack

    mov word [ebp - 6], (kernelInitialGdtEnd - kernelInitialGdt - 1) ; GDT size
    mov dword [ebp - 4], kernelInitialGdt ; GDT base
    lgdt [ebp - 6] ; Load GDT
    add esp, 6 ; Free stack

    ; Reload data segments
    mov ax, 0x0010
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Reload code segment
    jmp dword 0x0008:.reloadCodeSegment

.reloadCodeSegment:
    ; Return
    pop ebp
    ret

kernelInitPagingStructures:
    ; Initialize stack frame
    push ebp
    mov ebp, esp

    ; Save EDI
    push edi

    ; Compute the number of pages in the kernel
    mov ecx, g_kernelEnd
    sub ecx, g_kernelStart
    add ecx, 0x00000fff
    shr ecx, 12

    ; Make EDI point to page table entry 256 (1 MB)
    mov edi, kernelInitialPageTable + 256 * 4

    ; Prepare page table entry
    mov eax, 0x00100003

.fillPageTableLoop:
    ; Store page table entry
    stosd

    ; Increment the page table entry
    add eax, 0x00001000

    ; Loop until all kernel pages are mapped
    loop .fillPageTableLoop

.fillPageDirectory:
    ; Prepare the page directory entry
    mov eax, kernelInitialPageTable
    or eax, 0x00000003

    ; Set the entry 0 of the page directory (lower-half)
    mov dword [kernelInitialPageDirectory], eax

    ; Set the entry 768 of the page directory (higher-half)
    mov dword [kernelInitialPageDirectory + 768 * 4], eax

    ; Restore EDI
    pop edi

    ; Return
    pop ebp
    ret

enablePaging:
    ; Set page directory address
    mov eax, kernelInitialPageDirectory
    mov cr3, eax

    ; Enable paging bit in CR0
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; Return
    ret

section .data_low
align 0x1000
kernelInitialStackTop:
    times 0x1000 db 0
kernelInitialStackBottom:

kernelInitialPageDirectory:
    times 0x1000 db 0

kernelInitialPageTable:
    times 0x1000 db 0

section .rodata_low
align 8
kernelInitialGdt:
    ; GDT entry 0: NULL
    dq 0

    ; GDT entry 1: Code segment
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00

    ; GDT entry 2: Data segment
    db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00
kernelInitialGdtEnd:
