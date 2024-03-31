#ifdef _KERNEL_TARGET_ARCH_X86_64

#include "kernel/arch/x86_64/gdt.h"
#include "kernel/arch/x86_64/idt.h"
#include "kernel/arch/x86_64/inline.h"
#include "kernel/arch/x86_64/pic.h"
#include "kernel/drivers/block/pci_ide.h"
#include "kernel/drivers/bus/pci.h"
#include "klibc/debug.h"

int archPreInit(void) {
    gdtInit();
    idtInit();
    picInit();

    cli();

    return 0;
}

int archInit(void) {

}

int archPostInit() {
    if(pciInit() != 0) {
        kernelDebug("Warning: PCI initialization failed.\n");
        // TODO: consequences?
    } else {
        pciideInit();
    }

    return 0;
}

#endif
