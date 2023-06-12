#ifdef _KERNEL_TARGET_ARCH_X86_64

#include "kernel/arch/x86_64/gdt.h"
#include "kernel/arch/x86_64/idt.h"
#include "kernel/arch/x86_64/inline.h"
#include "kernel/arch/x86_64/pic.h"

int archPreinit(void) {
    gdtInit();
    idtInit();
    picInit();

    cli();

    return 0;
}

int archInit(void) {
    return 0;
}

#endif
