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

    while(1) {
        asm("hlt");
    }
}
