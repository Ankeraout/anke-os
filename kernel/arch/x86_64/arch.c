#ifdef _KERNEL_TARGET_ARCH_X86_64

#include "kernel/arch/x86_64/gdt.h"
#include "kernel/arch/x86_64/idt.h"
#include "kernel/arch/x86_64/inline.h"
#include "kernel/arch/x86_64/pic.h"
#include "kernel/drivers/bus/pci.h"
#include "klibc/debug.h"

int archPreinit(void) {
    gdtInit();
    idtInit();
    picInit();

    cli();

    return 0;
}

int archInit(void) {
    if(pciInit() != 0) {
        kernelDebug("Warning: PCI initialization failed.\n");
        // TODO: consequences?
    }

    return 0;
}

#endif
