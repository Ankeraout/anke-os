#include <stdbool.h>
#include <stdlib.h>

#include <kernel/arch/arch.h>
#include <kernel/boot/boot.h>
#include <kernel/fonts/fonts.h>
#include <kernel/fs/devfs.h>
#include <kernel/fs/vfs.h>
#include <kernel/debug.h>
#include <kernel/module.h>
#include <modules/framebuffer.h>

static int initializeFramebuffer(
    const struct ts_bootFramebuffer *p_framebuffer
);

void main(struct ts_boot *p_boot) {
    debug("kernel: Starting AnkeKernel...\n");

    // Initialize memory management
    if(archPreinit(p_boot) != 0) {
        debug("kernel: Architecture-specific initialization failed.\n");
        debug("kernel: System halted.\n");
        archHaltAndCatchFire();
    }

    // Initialize VFS
    if(vfsInit() != 0) {
        debug("kernel: Failed to initialize VFS subsystem.\n");
        archHaltAndCatchFire();
    }

    // Mounts
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

    // Initialize modules
    if(moduleInit() != 0) {
        debug("kernel: Failed to initialize module subsystem.\n");
        archHaltAndCatchFire();
    }

    // Initialize framebuffer
    initializeFramebuffer(&p_boot->a_framebuffer);

    // Initialize arch-specific devices.
    if(archInit() != 0) {
        debug("kernel: Architecture-specific initialization failed.\n");
        debug("kernel: System halted.\n");
        archHaltAndCatchFire();
    }

    debug("kernel: Done.\n");

    archHaltAndCatchFire();
}

static int initializeFramebuffer(
    const struct ts_bootFramebuffer *p_framebuffer
) {
    // Load framebuffer module
    debug("kernel: Loading framebuffer module...\n");

    const struct ts_module *l_moduleFramebuffer =
        moduleGetKernelModule("framebuffer");

    if(l_moduleFramebuffer == NULL) {
        debug("kernel: framebuffer module was not found.\n");
        return 1;
    }

    if(moduleLoad(l_moduleFramebuffer, NULL) != 0) {
        debug("kernel: Failed to load framebuffer module.\n");
        return 1;
    }

    // Open framebuffer driver
    struct ts_vfsFileDescriptor *l_framebuffer = vfsOpen("/dev/fb", 0);

    if(l_framebuffer == NULL) {
        debug("kernel: Failed to open framebuffer driver file.\n");
        return 1;
    }

    // Create framebuffer device
    struct ts_framebufferRequestCreate l_requestCreate = {
        .a_buffer = p_framebuffer->a_buffer,
        .a_width = p_framebuffer->a_width,
        .a_height = p_framebuffer->a_height,
        .a_pitch = p_framebuffer->a_pitch
    };

    if(
        l_framebuffer->a_ioctl(
            l_framebuffer,
            E_IOCTL_FRAMEBUFFER_CREATE,
            &l_requestCreate
        ) != 0
    ) {
        debug("kernel: Failed to create framebuffer device file.\n");
        return 1;
    }

    return 0;
}
