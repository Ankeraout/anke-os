bits 64

section .text

global criticalSection_enter
criticalSection_enter:
    cmp qword [s_reentrantCount], 0
    jnz .incrementReentrantCount

    .checkInterrupts:
        pushf
        pop rax
        and ah, 0x02
        shr ah, 1

        cli

        mov [s_wereInterruptsEnabled], ah

    .incrementReentrantCount:
        inc qword [s_reentrantCount]

    .end:
        ret

global criticalSection_leave
criticalSection_leave:
    cmp qword [s_reentrantCount], 1
    jnz .decrementCriticalSectionCount

    .restoreInterruptFlag:
        test byte [s_wereInterruptsEnabled], 1
        jz .decrementCriticalSectionCount

        sti

    .decrementCriticalSectionCount:
        dec qword [s_reentrantCount]

    .end:
        ret

section .bss
s_reentrantCount: resq 1
s_wereInterruptsEnabled: resq 1
