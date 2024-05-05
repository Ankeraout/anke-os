#include "kernel/printk.h"
#include "kernel/mm/pmm.h"

#include "kernel/arch/x86_64/gdt.h"
#include "kernel/arch/x86_64/idt.h"
#include "kernel/arch/x86_64/isr.h"
#include "kernel/mm/pmm.h"
#include "kernel/mm/vmm.h"
#include "kernel/panic.h"

void main(void) {
    printk("AnkeOS kernel\n");

    gdtInit();
    isrInit();
    idtInit();

    if(vmmInit() != 0) {
        panic("VMM initialization failed.\n");
    }

    pr_info("kernel: Initialization complete.\n");

    while(1) {
        asm("hlt");
    }
}
