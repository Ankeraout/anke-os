#include "arch/amd64/asm.h"
#include "arch/amd64/gdt.h"
#include "arch/amd64/idt.h"
#include "arch/amd64/pic.h"
#include "irq.h"
#include "mm/vmm.h"
#include "printk.h"

void arch_init(void) {
    if(vmmInit() != 0) {
        pr_crit("arch_init: Failed to initialize VMM.\n");
        
        while(1) {
            cli();
            hlt();
        }
    }

    gdtInit();
    irq_init();
    picInit();
    idtInit();

    sti();
}
