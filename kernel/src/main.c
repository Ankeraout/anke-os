#include "acpi/acpi.h"
#include "arch/arch.h"
#include "arch/amd64/gdt.h"
#include "bootstrap.h"
#include "printk.h"
#include "arch/amd64/pic.h"
#include "irq.h"

static void irqHandler(void *p_arg) {
    printk("IRQ0 ");
}

void main(void) {
    bootstrap();
    arch_init();

    irq_addHandler(0, irqHandler, NULL);
    picEnableIrq(0);

    while(1) {
        asm("hlt");
    }
}
