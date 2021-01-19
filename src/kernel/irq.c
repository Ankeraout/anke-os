#include <stddef.h>

#include "irq.h"

extern irq_handler_t *irq_handlers[];
extern void *irq_handlers_args[];

void irq_wrapper(int irq) {
    if(irq_handlers[irq]) {
        irq_handlers[irq](irq_handlers_args[irq]);
    }
}

void irq_init() {
    for(int i = 0; i < irq_getIrqLineCount(); i++) {
        irq_handlers[i] = NULL;
    }
}

void irq_register(int irq, irq_handler_t *handler, void *arg) {
    if(irq >= irq_getIrqLineCount()) {
        return;
    }

    irq_handlers[irq] = handler;
    irq_handlers_args[irq] = arg;
}
