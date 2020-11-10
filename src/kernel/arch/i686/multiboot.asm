%define MULTIBOOT_FLAG_MODULEALIGN (1 << 0)
%define MULTIBOOT_FLAG_MEM (1 << 1)
%define MULTIBOOT_FLAG_VIDEO (1 << 2)
%define MULTIBOOT_FLAG_RELOCATE (1 << 16)

%define MULTIBOOT_MAGIC 0x1badb002
%define MULTIBOOT_FLAGS (MULTIBOOT_FLAG_MODULEALIGN | MULTIBOOT_FLAG_MEM)
%define MULTIBOOT_CHECKSUM -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

%define KERNEL_STACK_SIZE 16384

%define VIRTUAL_TO_PHYSICAL_ADDR(address) (address - 0xc0000000)

section .multiboot.data
align 4
saved_eax: dd 0

section .multiboot.text

;Multiboot header
align 4
dd MULTIBOOT_MAGIC
dd MULTIBOOT_FLAGS
dd MULTIBOOT_CHECKSUM

global _start
extern kernel_main
_start:
.saveRegisters:
    ; Save EAX and EBX for later
    mov [saved_eax], eax

.copyMultibootInfo:
    mov edi, VIRTUAL_TO_PHYSICAL_ADDR(kernel_multibootInfo)
    mov esi, ebx
    mov ecx, 116 << 2
    repz movsd

.copyMemoryMap:
    ; Determine memory map length
    lea ebp, [ebx + 44]
    mov ecx, [ebp]
    cmp ecx, 4096
    ja .copyMemoryMap_shrink

.copyMemoryMap_definiteSize:
    mov [VIRTUAL_TO_PHYSICAL_ADDR(kernel_memoryMapLength)], ecx
    mov esi, [ebp + 4]
    mov edi, VIRTUAL_TO_PHYSICAL_ADDR(kernel_memoryMap)
    shr ecx, 2
    repz movsd
    jmp .fillPageDirectory

.copyMemoryMap_shrink:
    mov ecx, 4080
    jmp .copyMemoryMap_definiteSize

    ; Map the page table at index 0 and 768 of the page directory.
    ; Index 0 will map 0x00000000-0x003fffff and index 768 will map 0xc0000000-
    ; 0xc03fffff. 
.fillPageDirectory:
    mov ebx, VIRTUAL_TO_PHYSICAL_ADDR(kernel_pageDirectory)
    mov eax, VIRTUAL_TO_PHYSICAL_ADDR(kernel_bootstrapPageTable)
    or eax, 0x0000000f
    mov [ebx], eax
    mov [ebx + 4 * 768], eax

    ; Fill the page table
.fillPageTable:
    mov ebx, VIRTUAL_TO_PHYSICAL_ADDR(kernel_bootstrapPageTable)
    mov eax, 0x0000000f
    xor ecx, ecx

.fillPageTable_loop:
    mov [ebx + ecx * 4], eax
    inc ecx
    cmp ecx, 1024
    je .fillPageTable_finished
    add eax, 0x00001000
    jmp .fillPageTable_loop

.fillPageTable_finished:
.enablePaging:
    mov eax, VIRTUAL_TO_PHYSICAL_ADDR(kernel_pageDirectory)
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

.loadSavedRegisters:
    mov edx, [saved_eax] ;We load it in EDX for now because EAX is used later.

.jumpToHigherHalf:
    lea ecx, [.higherHalfStart]
    jmp ecx

section .text
.higherHalfStart:
.unmapLowerMemory:
    mov dword [kernel_pageDirectory], 0

.setupStack:
    mov esp, kernel_stack_top

.reloadGDT:
    lgdt [kernel_gdtr]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp dword 0x08:.restoreRegisters

.restoreRegisters:
    mov eax, edx

.callKernel:
    push eax
    call kernel_main

.halt:
    cli
    hlt
    jmp .halt

section .bss
align 16
kernel_stack:
kernel_stack_top:
    resb KERNEL_STACK_SIZE
kernel_stack_bottom:

align 4096
global kernel_pageDirectory
kernel_pageDirectory:
    resb 4096

kernel_bootstrapPageTable:
    resb 4096

align 4
global kernel_memoryMap
kernel_memoryMap:
    resb 4096

align 4
global kernel_memoryMapLength
kernel_memoryMapLength:
    resd 1

align 4
global kernel_multibootInfo
kernel_multibootInfo:
    resb 116

section .data
global kernel_gdt
kernel_gdt:
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
.tss:
    times 8 db 0
.codeSegment16:
    times 8 db 0
.dataSegment16:
    times 8 db 0
.end:

kernel_gdtr:
.limit:
    dw (kernel_gdt.end - kernel_gdt) - 1
.base:
    dd kernel_gdt
