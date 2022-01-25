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
    dd 640 ; Width
    dd 480 ; Height
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
    ; Save status
    mov [kernelBootStatus.eax], eax
    mov [kernelBootStatus.ebx], ebx

    ; Initialize lower-half kernel stack
    mov esp, kernelInitialStackBottom

    ; Initialize a basic flat GDT
    call kernelLoadInitialGdt

    ; Initialize paging structures
    call kernelInitPagingStructures

    ; Copy memory map from GRUB
    call kernelCopyMemoryMap

    ; Enable paging
    call enablePaging

    ; The stack has now moved to higher-half
    add esp, 0xc0000000

    ; Restore EAX and EBX
    lea ebp, [kernelBootStatus + 0xc0000000]
    push dword [ebp + 4]
    push dword [ebp]

    ; Jump to higher-half
    call kernelMain

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
    mov dword [g_kernelPageDirectoryLow], eax

    ; Set the entry 768 of the page directory (higher-half)
    mov dword [g_kernelPageDirectoryLow + 768 * 4], eax

    ; Restore EDI
    pop edi

.mapPageTables:
    ; Map self-mapping page table
    mov eax, g_kernelSelfMapPageTableLow

    or eax, 0x00000003
    mov [g_kernelPageDirectoryLow + 1023 * 4], eax

    ; Map page table 0
    mov eax, kernelInitialPageTable
    or eax, 0x00000003
    mov [g_kernelSelfMapPageTableLow], eax

    ; Map page table 768
    mov [g_kernelSelfMapPageTableLow + 768 * 4], eax

    ; Map page table 1023
    mov eax, g_kernelSelfMapPageTableLow

    or eax, 0x00000003
    mov [g_kernelSelfMapPageTableLow + 1023 * 4], eax

.return:
    ; Return
    pop ebp
    ret

enablePaging:
    ; Set page directory address
    mov eax, g_kernelPageDirectoryLow
    mov cr3, eax

    ; Enable paging bit in CR0
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; Return
    ret

kernelCopyMemoryMap:
    push ebp
    mov ebp, esp

    push esi
    push edi

    ; Find the memory map in the Multiboot 2 structure
    mov esi, [kernelBootStatus.ebx]

    ; We do not care about the first 8 bytes of the structure
    add esi, 8

.checkTag:
    mov eax, [esi]

    ; If the tag type is C_MULTIBOOT2_INFO_ID_MEMORY_MAP, get out of the
    ; loop
    cmp eax, C_MULTIBOOT2_INFO_ID_MEMORY_MAP
    jz .foundTag

    ; Get the tag size
    mov eax, [esi + 4]
    add esi, eax

    ; If the tag size is not 64-bits aligned, align it.
    test eax, 0x00000007
    jnz .alignNextTag

    jmp .checkTag

.alignNextTag:
    mov edx, eax
    and edx, 0x00000007
    mov eax, 8
    sub eax, edx
    add esi, eax
    jmp .checkTag

.foundTag:
    ; Compute the memory map size
    mov ecx, [esi + 4]
    sub ecx, 16

    ; Save the memory map size
    mov [g_kernelMemoryMapSizeLow], ecx

    shr ecx, 2

    ; Copy the memory map
    add esi, 16
    mov edi, g_kernelMemoryMapLow
    repz movsd

    ; Return
    pop edi
    pop esi

    pop ebp
    ret

section .data_low
align 0x1000
kernelInitialStackTop:
    times 0x1000 db 0
kernelInitialStackBottom:

global g_kernelPageDirectoryLow
g_kernelPageDirectoryLow:
    times 0x1000 db 0

kernelInitialPageTable:
    times 0x1000 db 0

global g_kernelSelfMapPageTableLow
g_kernelSelfMapPageTableLow:
    times 0x1000 db 0

global g_kernelMemoryMapLow
g_kernelMemoryMapLow:
    times 0x1000 db 0

global g_kernelMemoryMapSizeLow
g_kernelMemoryMapSizeLow:
    dd 0

kernelBootStatus:
.eax: dd 0
.ebx: dd 0

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
