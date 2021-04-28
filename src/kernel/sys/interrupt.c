#include "kernel/sys/interrupt.h"

extern interrupt_handler_t interrupt_handlers[];
extern void *interrupt_handlers_args[];

void interrupt_register(int line, interrupt_handler_t handler, void *arg);

void interrupt_handle(int line) {
    if(interrupt_handlers[line]) {
        interrupt_handlers[line](interrupt_handlers_args[line]);
    }
}

void interrupt_register(int line, interrupt_handler_t handler, void *arg) {
    interrupt_handlers[line] = handler;
    interrupt_handlers_args[line] = arg;
}
