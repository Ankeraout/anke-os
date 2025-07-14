#ifndef __INCLUDE_IRQ_H__
#define __INCLUDE_IRQ_H__

#define C_IRQ_MAX 16

typedef void tf_irqHandler(void *p_arg);

int irq_init(void);
int irq_addHandler(int p_irq, tf_irqHandler *p_handler, void *p_arg);
void irq_service(int p_irq);
int irq_removeHandler(int p_irq, tf_irqHandler *p_handler, void *p_arg);
void irq_endOfInterrupt(int p_irq);

#endif
