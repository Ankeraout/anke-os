#include <stddef.h>
#include <stdint.h>

#include "irq.h"
#include "arch/i686/io.h"

#define TIMER_FREQUENCY 1193182
#define DESIRED_FREQUENCY 100
#define DIVIDER (TIMER_FREQUENCY / DESIRED_FREQUENCY)

void pit_init();
static void pit_irq();

volatile uint64_t kernel_timer = 0;

void pit_init() {
    uint16_t divider = (uint16_t)DIVIDER;

    outb(0x43, 0x04);
    outb(0x40, divider);
    outb(0x40, divider >> 8);

    irq_register(0, pit_irq, NULL);
}

static void pit_irq() {
    kernel_timer += 10;
}
