#include "kernel/module.h"
#include "klibc/debug.h"

int init(void) {
    kernelDebug("pci: Hello world!\n");
    return 0;
}

void exit(void) {
    kernelDebug("pci: Exiting!\n");
}

M_DECLARE_MODULE("pci", init, exit);
