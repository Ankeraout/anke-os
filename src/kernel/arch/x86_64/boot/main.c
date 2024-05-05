#include "kernel/printk.h"
#include "kernel/mm/pmm.h"

#include "kernel/arch/x86_64/asm.h"
#include "kernel/arch/x86_64/gdt.h"
#include "kernel/arch/x86_64/idt.h"
#include "kernel/arch/x86_64/isr.h"
#include "kernel/arch/x86_64/pic.h"
#include "kernel/mm/pmm.h"
#include "kernel/mm/vmm.h"
#include "kernel/panic.h"

void main(void) {
    printk("AnkeOS kernel\n");

    gdtInit();
    isrInit();
    idtInit();
    picInit();

    if(vmmInit() != 0) {
        panic("kernel: VMM initialization failed.\n");
    }

    pr_info("kernel: Initialization complete.\n");

    sti();

    while(1) {
        hlt();
    }
}
