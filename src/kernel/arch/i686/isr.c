#include "arch/i686/io.h"

#include "libk/libk.h"

__attribute__((interrupt)) void isr_handler0_7(void *arg) {
    UNUSED_PARAMETER(arg);
    outb(0x20, 0x20);
}

__attribute__((interrupt)) void isr_handler8_15(void *arg) {
    UNUSED_PARAMETER(arg);
    outb(0xa0, 0x20);
    outb(0x20, 0x20);
}
