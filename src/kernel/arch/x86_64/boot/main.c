#include "kernel/printk.h"
#include "kernel/mm/pmm.h"

#include "kernel/arch/x86_64/gdt.h"
#include "kernel/arch/x86_64/idt.h"

void _start(void) {
    printk("AnkeOS kernel\n");

    gdtInit();
    idtInit();

    while(1) {
        asm("hlt");
    }
}
