#ifndef __KERNEL_ARCH_ARCH_HPP__
#define __KERNEL_ARCH_ARCH_HPP__

namespace kernel {
    // Arch-specific initialization functions
    void arch_preinit();
    void arch_init();

    // Arch-specific functions
    void halt();
}

#endif
