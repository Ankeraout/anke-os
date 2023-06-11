#ifdef _KERNEL_TARGET_ARCH_X86_64

#include "kernel/arch/x86_64/gdt.h"

int archPreinit(void) {
    gdtInit();
    return 0;
}

int archInit(void) {
    return 0;
}

#endif
