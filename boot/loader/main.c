#include "arch/x86/asm.h"
#include "arch/x86/bioscall.h"
#include "arch/x86/idt.h"
#include "arch/x86/irq.h"
#include "drivers/irq/i8259.h"
#include "stdio.h"

static void irqHandler0(void *p_arg) {
    putchar('+');
    outb(0x20, 0x20);
}

static void irqHandler1(void *p_arg) {
    putchar('.');
    inb(0x60);
    outb(0x20, 0x20);
}

void main(void) {
    stdio_init();

    printf("Hello from protected mode.\n");

    printf("Loading IDT...\n");
    idtInit();

    printf("Initializing i8259...\n");
    i8259_init();
    irqAdd(32, irqHandler0, NULL);
    irqUnmask(32);
    irqAdd(33, irqHandler1, NULL);
    irqUnmask(33);

    printf("Enabling interrupts...\n");
    sti();

    while(1) {
        hlt();
    }
}
