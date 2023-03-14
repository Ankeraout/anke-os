#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/arch/arch.h>
#include <kernel/boot/boot.h>
#include <kernel/dev/device.h>
#include <kernel/fonts/fonts.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/debug.h>
#include <kernel/module.h>
#include <modules/framebuffer.h>
#include <modules/tty.h>

void main(struct ts_boot *p_boot) {
    debug("kernel: Starting AnkeKernel...\n");

    // Initialize memory management
    if(archPreinit(p_boot) != 0) {
        debug("kernel: Architecture-specific preinitialization failed.\n");
        debug("kernel: System halted.\n");
        archHaltAndCatchFire();
    }

    // Initialize VFS
    if(vfsInit() != 0) {
        debug("kernel: Failed to initialize VFS subsystem.\n");
        archHaltAndCatchFire();
    }

    // Initialize device system
    if(deviceInit()) {
        debug("kernel: Failed to initialize device system.\n");
        archHaltAndCatchFire();
    }

    // Initialize modules
    if(moduleInit() != 0) {
        debug("kernel: Failed to initialize module subsystem.\n");
        archHaltAndCatchFire();
    }

    // Initialize arch-specific devices.
    if(archInit() != 0) {
        debug("kernel: Architecture-specific initialization failed.\n");
        debug("kernel: System halted.\n");
        archHaltAndCatchFire();
    }

    debug("kernel: Done.\n");

    archHaltAndCatchFire();
}
