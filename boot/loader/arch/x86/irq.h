#ifndef __INCLUDE_ARCH_X86_IRQ_H__
#define __INCLUDE_ARCH_X86_ISR_H__

#include <stdint.h>

#include "drivers/irq/irq_chip.h"

#define C_EXCEPTION_COUNT 32
#define C_IRQ_COUNT 224
#define C_INT_COUNT (C_EXCEPTION_COUNT + C_IRQ_COUNT)

struct ts_irqRegisters {
    uint32_t m_gs, m_fs, m_es, m_ds;
    uint32_t m_ebp, m_edi, m_esi;
    uint32_t m_edx, m_ecx, m_ebx, m_eax;
    uint32_t m_interruptNumber, m_errorCode;
    uint32_t m_eip, m_cs, m_eflags, m_esp, m_ss;
} __attribute__((packed));

typedef void tf_irqHandler(void *p_arg);

void irqHandler(struct ts_irqRegisters *p_registers);
void irqInit(void);
void irqAdd(int p_interrupt, tf_irqHandler *p_handler, void *p_arg);
void irqRemove(int p_interrupt, tf_irqHandler *p_handler, void *p_arg);
void irqSetChip(int p_interrupt, struct ts_irqChip *p_irqChip);
void irqMask(int p_interrupt);
void irqUnmask(int p_interrupt);

#endif
