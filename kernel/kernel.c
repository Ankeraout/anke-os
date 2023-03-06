#include <stdbool.h>
#include <stdlib.h>

#include <kernel/arch/arch.h>
#include <kernel/boot/boot.h>
#include <kernel/fonts/fonts.h>
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

    // Create a dummy file descriptor for /.
    struct ts_vfsFileDescriptor l_rootFileDescriptor = {
        .a_name = ""
    };

    // Create a dummy file descriptor for /dev.
    struct ts_vfsFileDescriptor l_devFileDescriptor = {
        .a_name = "dev"
    };

    vfsMount("/", &l_rootFileDescriptor);
    vfsMount("/dev", &l_devFileDescriptor);

    vfsDebug();

    debug("kernel: Getting /dev mount point...\n");

    const char *l_relativePath = NULL;
    struct ts_vfsFileDescriptor *l_devMountPoint =
        vfsGetMountPoint("/dev/sda", &l_relativePath);

    if(l_devMountPoint == NULL) {
        debug("kernel: /dev is not mounted.\n");
    } else {
        debug("kernel: /dev is mounted. Relative path: %s\n", l_relativePath);
    }

    archHaltAndCatchFire();
}
