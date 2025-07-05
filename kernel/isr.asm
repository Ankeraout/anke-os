section .text

%macro M_DEFINE_ISR_IRQ 1
isr_irq%1:
    mov byte [g_isr_irq], %1
    jmp irq_service
%endmacro

%assign l_irqNumber 0
%rep C_IRQ_COUNT
M_DEFINE_ISR_IRQ l_irqNumber
%assign l_irqNumber l_irqNumber + 1
%endrep
%undef l_irqNumber

section .bss
align 2, resb 1
g_isr_irq: resb 1
