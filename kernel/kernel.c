#include <stdbool.h>
#include <stdlib.h>

#include <kernel/arch/arch.h>
#include <kernel/boot/boot.h>
#include <kernel/fonts/fonts.h>
#include <kernel/fs/devfs.h>
#include <kernel/fs/vfs.h>
#include <kernel/debug.h>
#include <kernel/module.h>

void main(struct ts_boot *p_boot) {
    debug("kernel: Starting AnkeKernel...\n");

    if(archPreinit(p_boot) != 0) {
        debug("kernel: Architecture-specific initialization failed.\n");
        debug("kernel: System halted.\n");
        archHaltAndCatchFire();
    }

    if(vfsInit() != 0) {
        debug("kernel: Failed to initialize VFS subsystem.\n");
        archHaltAndCatchFire();
    }

    debug("kernel: Mounting /dev...\n");

    struct ts_vfsFileDescriptor *l_devFileDescriptor = devfsInit();

    if(l_devFileDescriptor == NULL) {
        debug("panic: Failed to create /dev.\n");
        archHaltAndCatchFire();
    }

    if(vfsMount("/dev", l_devFileDescriptor) != 0) {
        debug("panic: Failed to mount /dev.\n");
        archHaltAndCatchFire();
    }

    vfsDebug();

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
