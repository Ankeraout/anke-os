bits 64

section .text

global spinlock_acquire
spinlock_acquire:
    test qword [rdi], 1
    jnz .alreadyLocked

    .notLocked:
        lock bts qword [rdi], 0
        jc .alreadyLocked
    
    .acquired:
        ret

    .alreadyLocked:
        pause
        jmp spinlock_acquire

global spinlock_release
spinlock_release:
    mov qword [rdi], 0
    ret
