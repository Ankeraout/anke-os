#include "arch/arch.h"
#include "bootstrap.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "panic.h"
#include "printk.h"

void main(void) {
    arch_init();

    const struct ts_bootstrap_information *l_bootstrapInformation =
        bootstrap_getInformation();

    g_pmm_hhdm = l_bootstrapInformation->m_hhdm;

    printk("AnkeKernel 0.1.0\n");
    printk("Compiled " __DATE__ " " __TIME__ "\n");

    if(pmm_init()) {
        panic("pmm_init() failed.\n");
    }

    if(vmm_init()) {
        panic("vmm_init() failed.\n");
    }

    while(1) {
        asm("hlt");
    }
}
