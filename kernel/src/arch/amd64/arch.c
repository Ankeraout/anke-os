#include "arch/amd64/asm.h"
#include "arch/amd64/gdt.h"
#include "arch/amd64/idt.h"
#include "arch/amd64/pic.h"
#include "irq.h"
#include "mm/vmm.h"
#include "printk.h"

void arch_init(void) {
    if(vmm_init() != 0) {
        pr_crit("arch_init: Failed to initialize VMM.\n");
        
        while(1) {
            asm_cli();
            asm_hlt();
        }
    }

    gdt_init();
    irq_init();
    pic_init();
    idt_init();

    asm_sti();
}
