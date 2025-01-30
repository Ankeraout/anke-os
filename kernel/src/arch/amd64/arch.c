#include "arch/amd64/asm.h"
#include "arch/amd64/gdt.h"
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
}
