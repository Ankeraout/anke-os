#include <stdbool.h>
#include <stddef.h>

#include <kernel/arch/arch.h>
#include <kernel/arch/x86_64/gdt.h>
#include <kernel/arch/x86_64/idt.h>
#include <kernel/arch/x86_64/inline.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/mm/pmm.h>
#include <kernel/mm/vmm.h>
#include <kernel/debug.h>
#include <kernel/module.h>

static int archInitPci(void);
static int archInitPciide(void);

int archPreinit(struct ts_boot *p_boot) {
    gdtInit();
    idtInit();

    if(pmmInit(p_boot->a_memoryMap, p_boot->a_memoryMapLength) != 0) {
        debug("kernel: Physical memory manager initialization failed.\n");
        return 1;
    }

    return 0;
}

int archInit(void) {
    archInitPci();
    archInitPciide();
    return 0;
}

void archInterruptsEnable(void) {
    sti();
}

void archInterruptsDisable(void) {
    cli();
}

void archHalt(void) {
    hlt();
}

void archHaltAndCatchFire(void) {
    while(true) {
        cli();
        hlt();
    }
}

static int archInitPci(void) {
    // Find PCI module
    const struct ts_module *l_pciModule = moduleGetKernelModule("pci");

    if(l_pciModule == NULL) {
        debug("kernel: pci module not found.\n");
        return 1;
    }

    // Load PCI module
    if(moduleLoad(l_pciModule, NULL) != 0) {
        debug("kernel: pci module initialization failed.\n");
        return 1;
    }

    debug("kernel: pci module was initialized successfully.\n");

    return 0;
}

static int archInitPciide(void) {
    // Find pciide module
    const struct ts_module *l_pciideModule = moduleGetKernelModule("pciide");

    if(l_pciideModule == NULL) {
        debug("kernel: pciide module not found.\n");
        return 1;
    }

    if(moduleLoad(l_pciideModule, NULL) != 0) {
        debug("kernel: pciide module initialization failed.\n");
        return 1;
    }

    debug("kernel: pciide module was initialized successfully.\n");

    return 0;
}
