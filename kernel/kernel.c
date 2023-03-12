#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/arch/arch.h>
#include <kernel/boot/boot.h>
#include <kernel/fonts/fonts.h>
#include <kernel/fs/devfs.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/debug.h>
#include <kernel/module.h>
#include <modules/framebuffer.h>
#include <modules/tty.h>

static int initializeFramebuffer(
    const struct ts_bootFramebuffer *p_framebuffer
);
static int initializeTty(const char *p_fbDeviceName);
static void ttyDebug(void *p_parameter, const char *p_buffer);

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

    // Initialize modules
    if(moduleInit() != 0) {
        debug("kernel: Failed to initialize module subsystem.\n");
        archHaltAndCatchFire();
    }

    // Initialize framebuffer
    if(initializeFramebuffer(&p_boot->a_framebuffer) == 0) {
        initializeTty("/dev/fb0");
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
    struct ts_vfsFileDescriptor *l_framebuffer = vfsFind("/dev/fb");

    if(l_framebuffer == NULL) {
        debug("kernel: Failed to find framebuffer driver file.\n");
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
        kfree(l_framebuffer);
        debug("kernel: Failed to create framebuffer device file.\n");
        return 1;
    }

    kfree(l_framebuffer);

    return 0;
}

static int initializeTty(const char *p_fbDeviceName) {
    // Load tty module
    debug("kernel: Loading tty module...\n");

    const struct ts_module *l_moduleTty = moduleGetKernelModule("tty");

    if(l_moduleTty == NULL) {
        debug("kernel: tty module was not found.\n");
        return 1;
    }

    if(moduleLoad(l_moduleTty, NULL) != 0) {
        debug("kernel: Failed to load tty module.\n");
        return 1;
    }

    // Open tty driver
    struct ts_vfsFileDescriptor *l_tty = vfsFind("/dev/tty");

    if(l_tty == NULL) {
        debug("kernel: Failed to find tty driver file.\n");
        return 1;
    }

    // Create tty device
    int l_returnValue = l_tty->a_ioctl(
        l_tty,
        E_IOCTL_TTY_CREATE,
        (void *)p_fbDeviceName
    );

    kfree(l_tty);

    if(l_returnValue != 0) {
        debug("kernel: Failed to create /dev/tty0.\n");
        return l_returnValue;
    }

    debug("kernel: Created /dev/tty0.\n");

    l_tty = vfsFind("/dev/tty0");

    // Set kernel debug function
    debugInit(ttyDebug, l_tty);

    debug("kernel: Now printing on /dev/tty0.\n");

    return 0;
}

static void ttyDebug(void *p_parameter, const char *p_buffer) {
    struct ts_vfsFileDescriptor *l_tty =
        (struct ts_vfsFileDescriptor *)p_parameter;

    l_tty->a_write(l_tty, p_buffer, strlen(p_buffer));
}
