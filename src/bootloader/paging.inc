bits 32

section .text
paging_init:
    ; Enable paging
    mov eax, .pageDirectory
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ret

section .data
.pageDirectory:
    align 4096
    times 4096 db 0