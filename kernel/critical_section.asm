section .text

; void criticalSection_enter()
criticalSection_enter:
    push bp
    mov bp, sp

    cmp word [g_criticalSection_reentrantCount], 0
    jnz .incrementCriticalSectionCount

.checkInterrupts:
    pushf
    pop ax
    and ah, 0x02
    shr ah, 1

    cli

    mov [g_criticalSection_wereInterruptsEnabled], ah

.incrementCriticalSectionCount:
    inc word [g_criticalSection_reentrantCount]
    
.end:
    pop bp
    ret

; void criticalSection_leave()
criticalSection_leave:
    push bp
    mov bp, sp

    cmp word [g_criticalSection_reentrantCount], 1
    jnz .decrementCriticalSectionCount

.restoreInterruptFlag:
    test byte [g_criticalSection_wereInterruptsEnabled], 0x01
    jz .decrementCriticalSectionCount

    sti

.decrementCriticalSectionCount:
    dec word [g_criticalSection_reentrantCount]

    pop bp
    ret

section .data
align 2
g_criticalSection_reentrantCount: dw 0

section .bss
align 2, resb 1
g_criticalSection_wereInterruptsEnabled: resw 1
