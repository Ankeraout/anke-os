#ifndef __KERNEL_ARCH_ARCH_H__
#define __KERNEL_ARCH_ARCH_H__

/**
 * \brief Architecture-specific initialization function. This function is
 * responsible for initializing the memory manager.
 */
void arch_preinit();

/**
 * \brief Architecture-specific initialization function. This function is
 * responsible for system device detection.
 */
void arch_init();

// Arch-specific functions
void arch_disableInterrupts();
void arch_halt();

#endif
