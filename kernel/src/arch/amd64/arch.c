#include "arch/amd64/asm.h"
#include "arch/amd64/gdt.h"
#include "arch/amd64/idt.h"
#include "arch/amd64/pic.h"
#include "arch/amd64/tss.h"
#include "irq.h"
#include "mm/vmm.h"
#include "printk.h"

#define C_KERNEL_STACK_SIZE 4096

static __attribute__((aligned(8))) uint8_t s_kernelStack[C_KERNEL_STACK_SIZE];

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
    tss_initTss(&g_tss[0], (size_t)s_kernelStack);
    asm_ltr(0x20);

    asm_sti();
}
