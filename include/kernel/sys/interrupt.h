#ifndef __KERNEL_IRQ_H__
#define __KERNEL_IRQ_H__

typedef void (*interrupt_handler_t)();

void interrupt_register(int line, interrupt_handler_t handler, void *arg);

#endif
