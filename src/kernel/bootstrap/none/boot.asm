extern arch_bootstrap

bits 32

section .bootstrap

global _start
_start:
    jmp arch_bootstrap
