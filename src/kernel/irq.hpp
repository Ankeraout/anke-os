#ifndef __KERNEL_IRQ_HPP__
#define __KERNEL_IRQ_HPP__

namespace kernel {
    typedef void irq_handler_t(void *);

    extern "C" int irq_getIrqLineCount();
    extern void irq_init();
    extern void irq_register(int irq, irq_handler_t *handler, void *arg);
}

#endif
