bits 64

section .text

global spinlockInit
spinlockInit:
    mov dword [rdi], 0
    ret

global spinlockAcquire
spinlockAcquire:
    lock bts dword [rdi], 0
    jc .wait
    ret

    .wait:
        test dword [rdi], 1
        jnz .wait
        jmp spinlockAcquire

global spinlockRelease
spinlockRelease:
    lock btr dword [rdi], 0
    ret
