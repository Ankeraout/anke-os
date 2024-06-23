#include <stdint.h>

#include "drivers/irq/irq_chip.h"
#include "irq.h"
#include "stdio.h"
#include "string.h"

static tf_irqHandler *s_irqHandlers[C_INT_COUNT];
static void *s_irqHandlerArgs[C_INT_COUNT];
static struct ts_irqChip *s_irqChip[C_INT_COUNT];

void irqHandler(struct ts_irqRegisters *p_registers) {
    const uint32_t l_interruptNumber = p_registers->m_interruptNumber;

    if(l_interruptNumber >= C_INT_COUNT) {
        return;
    }

    tf_irqHandler *l_interruptHandler =
        s_irqHandlers[l_interruptNumber];
    struct ts_irqChip *l_irqChip = s_irqChip[l_interruptNumber];

    if(l_interruptHandler != NULL) {
        l_interruptHandler(s_irqHandlerArgs[l_interruptNumber]);
    }

    if(l_irqChip != NULL) {
        l_irqChip->m_endOfInterrupt((int)l_interruptNumber);
    }
}

void irqInit(void) {
    memset(s_irqHandlers, 0, sizeof(s_irqHandlers));
    memset(s_irqChip, 0, sizeof(s_irqChip));
}

void irqAdd(int p_interrupt, tf_irqHandler *p_handler, void *p_arg) {
    if(p_interrupt < 0 || p_interrupt >= C_INT_COUNT) {
        return;
    }

    s_irqHandlers[p_interrupt] = p_handler;
    s_irqHandlerArgs[p_interrupt] = p_arg;
}

void irqRemove(int p_interrupt, tf_irqHandler *p_handler, void *p_arg) {
    if(p_interrupt < 0 || p_interrupt >= C_INT_COUNT) {
        return;
    }

    s_irqHandlers[p_interrupt] = NULL;
}

void irqSetChip(int p_interrupt, struct ts_irqChip *p_irqChip) {
    if(p_interrupt < 0 || p_interrupt >= C_INT_COUNT) {
        return;
    }

    s_irqChip[p_interrupt] = p_irqChip;
}

void irqMask(int p_interrupt) {
    if(p_interrupt < 0 || p_interrupt >= C_INT_COUNT) {
        return;
    }

    struct ts_irqChip *l_irqChip = s_irqChip[p_interrupt];

    if(l_irqChip != NULL) {
        l_irqChip->m_mask(p_interrupt);
    }
}

void irqUnmask(int p_interrupt) {
    if(p_interrupt < 0 || p_interrupt >= C_INT_COUNT) {
        return;
    }

    struct ts_irqChip *l_irqChip = s_irqChip[p_interrupt];

    if(l_irqChip != NULL) {
        l_irqChip->m_unmask(p_interrupt);
    }
}
