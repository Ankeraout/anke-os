bits 64

section .text

global spinlockAcquire
spinlockAcquire:
    lock bts dword [edi], 0
    jc .wait
    ret

    .wait:
        test dword [edi], 1
        jnz .wait
        jmp spinlockAcquire

global spinlockRelease
spinlockRelease:
    lock btr dword [edi], 0
    ret
