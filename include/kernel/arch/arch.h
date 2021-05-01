#ifndef __KERNEL_ARCH_H__
#define __KERNEL_ARCH_H__

#include "kernel/arch/x86/assembly.h"

#define arch_halt hlt

void arch_preinit();
void arch_init();

#endif
