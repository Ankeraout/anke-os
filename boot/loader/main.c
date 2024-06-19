#include "arch/x86/asm.h"
#include "arch/x86/bioscall.h"
#include "arch/x86/idt.h"
#include "arch/x86/isr.h"
#include "arch/x86/pic.h"
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

    printf("Initializing PIC...\n");
    picInit();

    printf("Registering ISR...\n");
    isrAdd(32, irqHandler0, NULL);
    isrAdd(33, irqHandler1, NULL);

    printf("Enabling interrupts...\n");
    sti();

    while(1) {
        hlt();
    }
}
