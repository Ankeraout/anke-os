#ifndef __KERNEL_IRQ_H__
#define __KERNEL_IRQ_H__

typedef void irq_handler_t(void *);

int irq_getIrqLineCount();
void irq_init();
void irq_register(int irq, irq_handler_t *handler, void *arg);

#endif
