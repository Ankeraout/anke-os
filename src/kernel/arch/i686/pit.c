#include <stddef.h>
#include <stdint.h>

#include "irq.h"
#include "arch/i686/io.h"
#include "libk/stdio.h"

#define TIMER_FREQUENCY 1193182
#define DESIRED_FREQUENCY 1000
#define DIVIDER (TIMER_FREQUENCY / DESIRED_FREQUENCY)

void pit_init();
static void pit_irq();
void sleep(unsigned int milliseconds);

volatile uint64_t kernel_timer = 0;

void pit_init() {
    uint16_t divider = (uint16_t)DIVIDER;

    outb(0x43, 0x34);
    outb(0x40, divider);
    outb(0x40, divider >> 8);

    irq_register(0, pit_irq, NULL);
}

static void pit_irq() {
    kernel_timer += (1000 / DESIRED_FREQUENCY);
}

void sleep(unsigned int milliseconds) {
    if(milliseconds == 0) {
        return;
    }

    uint64_t endTime = kernel_timer + milliseconds + (1000 / DESIRED_FREQUENCY);

    while(kernel_timer < endTime) {
        hlt();
    }
}
