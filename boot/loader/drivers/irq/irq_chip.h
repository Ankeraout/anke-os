#ifndef __INCLUDE_DRIVERS_IRQ_CHIP_IRQ_CHIP_H__
#define __INCLUDE_DRIVERS_IRQ_CHIP_IRQ_CHIP_H__

struct ts_irqChip {
    void (*m_mask)(int p_irq);
    void (*m_unmask)(int p_irq);
    void (*m_endOfInterrupt)(int p_irq);
};

#endif
