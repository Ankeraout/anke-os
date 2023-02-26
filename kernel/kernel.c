#include <stdbool.h>

#include <kernel/arch/arch.h>
#include <kernel/boot/boot.h>
#include <kernel/fonts/fonts.h>
#include <kernel/debug.h>
#include <kernel/module.h>

void main(struct ts_boot *p_boot) {
    debug("kernel: Starting AnkeKernel...\n");

    if(archPreinit(p_boot) != 0) {
        debug("kernel: Architecture-specific initialization failed.\n");
        debug("kernel: System halted.\n");
        archHaltAndCatchFire();
    }

    // Here, memory management is available.

    if(moduleInit() != 0) {
        debug("kernel: Failed to initialize module subsystem.\n");
        archHaltAndCatchFire();
    }

    if(archInit() != 0) {
        debug("kernel: Architecture-specific initialization failed.\n");
        debug("kernel: System halted.\n");
        archHaltAndCatchFire();
    }

    debug("kernel: Done.\n");

    archHaltAndCatchFire();
}
