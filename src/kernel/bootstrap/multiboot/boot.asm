%define MULTIBOOT_HEADER_MAGIC 0x1badb002
%define MULTIBOOT_HEADER_FLAGS 0x00000000
%define MULTIBOOT_HEADER_CHECKSUM -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

extern arch_bootstrap

bits 32

section .multiboot

; Multiboot header
align 4
dd MULTIBOOT_HEADER_MAGIC
dd MULTIBOOT_HEADER_FLAGS
dd MULTIBOOT_HEADER_CHECKSUM

global _start
_start:
    jmp arch_bootstrap
