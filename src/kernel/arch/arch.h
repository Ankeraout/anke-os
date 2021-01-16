#ifndef __KERNEL_ARCH_ARCH_H__
#define __KERNEL_ARCH_ARCH_H__

// Arch-specific initialization function
void arch_init();

// Arch-specific functions
void arch_disableInterrupts();
void arch_halt();

#endif
