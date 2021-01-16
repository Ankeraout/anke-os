%define MULTIBOOT_FLAG_MODULEALIGN (1 << 0)
%define MULTIBOOT_FLAG_MEM (1 << 1)
%define MULTIBOOT_FLAG_VIDEO (1 << 2)
%define MULTIBOOT_FLAG_RELOCATE (1 << 16)

%define MULTIBOOT_MAGIC 0x1badb002
%define MULTIBOOT_FLAGS (MULTIBOOT_FLAG_MODULEALIGN | MULTIBOOT_FLAG_MEM)
%define MULTIBOOT_CHECKSUM -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot

;Multiboot header
align 4
dd MULTIBOOT_MAGIC
dd MULTIBOOT_FLAGS
dd MULTIBOOT_CHECKSUM

global _start
extern arch_bootstrap
_start:
    jmp arch_bootstrap
