#include "acpi/acpi.h"
#include "arch/arch.h"
#include "arch/amd64/gdt.h"
#include "bootstrap.h"
#include "printk.h"
#include "arch/amd64/pic.h"
#include "irq.h"

void main(void) {
    bootstrap();
    arch_init();

    printk("AnkeKernel 0.1.0\n");
    printk("Compiled " __DATE__ " " __TIME__ "\n");

    while(1) {
        asm("hlt");
    }
}
