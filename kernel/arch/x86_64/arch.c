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

const char *s_moduleList[] = {
    "pci",
    "pciide",
    "cmos"
};

static void archLoadModules(void);
static int archLoadModule(const char *p_moduleName);

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
    archLoadModules();
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

static void archLoadModules(void) {
    size_t l_nbModules = sizeof(s_moduleList) / sizeof(const char *);

    for(size_t l_index = 0; l_index < l_nbModules; l_index++) {
        archLoadModule(s_moduleList[l_index]);
    }
}

static int archLoadModule(const char *p_moduleName) {
    const struct ts_module *l_module = moduleGetKernelModule(p_moduleName);

    if(l_module == NULL) {
        debug("kernel: Module %s not found.\n", p_moduleName);
        return 1;
    }

    if(moduleIsModuleLoaded(l_module)) {
        debug("kernel: Module %s is already loaded.\n", p_moduleName);
        return 0;
    }

    if(moduleLoad(l_module, NULL) != 0) {
        debug("kernel: Initialization of module %s failed.\n", p_moduleName);
        return 1;
    }

    return 0;
}
