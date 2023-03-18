#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/arch/arch.h>
#include <kernel/boot/boot.h>
#include <kernel/common.h>
#include <kernel/dev/device.h>
#include <kernel/fonts/fonts.h>
#include <kernel/fs/ramfs.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/debug.h>
#include <kernel/module.h>
#include <modules/framebuffer.h>
#include <modules/tty.h>

static int kernelCreateRootDirectories(void);
static int kernelInitFramebuffer(void);
static int kernelInitTty(void);
static int kernelMountRootfs(void);
static void kernelDebugWrite(void *p_parameter, const char *p_value);

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

    // Mount root
    if(kernelMountRootfs()) {
        debug("kernel: Failed to mount root.\n");
        archHaltAndCatchFire();
    }

    if(kernelCreateRootDirectories()) {
        debug("kernel: Failed to create root directories.\n");
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

    if(kernelInitFramebuffer()) {
        debug("kernel: Failed to initialize framebuffer.\n");
        archHaltAndCatchFire();
    }

    if(kernelInitTty()) {
        debug("kernel: Failed to initialize tty.\n");
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

static int kernelCreateRootDirectories(void) {
    static const char *l_rootDirectories[] = {
        "dev"
    };

    struct ts_vfsNode *l_root;
    int l_returnValue = vfsLookup(NULL, "/", &l_root);

    if(l_returnValue != 0) {
        debug("kernel: VFS root node was not found.\n");
        return l_returnValue;
    }

    for(
        size_t l_index = 0;
        l_index < M_ARRAY_ELEMENT_COUNT(l_rootDirectories);
        l_index++
    ) {
        int l_returnValue =
            vfsOperationMkdir(l_root, l_rootDirectories[l_index]);

        if(l_returnValue != 0) {
            debug(
                "kernel: Failed to create /%s: %d\n",
                l_rootDirectories[l_index],
                l_returnValue
            );

            return l_returnValue;
        }
    }

    // Close the root node (should have no effect, but let's follow best
    // practices!)
    l_returnValue = vfsOperationClose(l_root);

    if(l_returnValue != 0) {
        debug("kernel: Failed to close root node.\n");
        return l_returnValue;
    }

    return 0;
}

static void kernelDebugWrite(void *p_parameter, const char *p_value) {
    size_t l_length = strlen(p_value);

    vfsOperationWrite(p_parameter, p_value, l_length);
}

static int kernelInitFramebuffer(void) {
    const struct ts_module *l_moduleFramebuffer =
        moduleGetKernelModule("framebuffer");

    if(l_moduleFramebuffer == NULL) {
        debug("kernel: \"framebuffer\" module was not found.\n");
        return 1;
    }

    if(moduleLoad(l_moduleFramebuffer, NULL) != 0) {
        debug("kernel: \"framebuffer\" module initialization failed.\n");
        return 1;
    }

    return 0;
}

static int kernelInitTty(void) {
    const struct ts_module *l_moduleTty =
        moduleGetKernelModule("tty");

    if(l_moduleTty == NULL) {
        debug("kernel: \"tty\" module was not found.\n");
        return 1;
    }

    if(moduleLoad(l_moduleTty, "/dev/fb0") != 0) {
        debug("kernel: \"tty\" module initialization failed.\n");
        return 1;
    }

    struct ts_vfsNode *l_tty0;
    int l_returnValue = vfsLookup(NULL, "/dev/tty0", &l_tty0);

    if(l_returnValue != 0) {
        debug("kernel: Failed to open /dev/tty0: %d.\n", l_returnValue);
        return 1;
    }

    debugInit(kernelDebugWrite, l_tty0);

    return 0;
}

static int kernelMountRootfs(void) {
    // Get the VFS node for "/".
    struct ts_vfsNode *l_root;
    int l_returnValue = vfsLookup(NULL, "/", &l_root);

    if(l_returnValue != 0) {
        debug("kernel: VFS root node was not found.\n");
        return l_returnValue;
    }

    // Mount a ramfs instance
    l_returnValue = vfsMount(l_root, &g_ramfsFileSystem);

    if(l_returnValue != 0) {
        debug("kernel: Failed to mount root as ramfs.\n");
        return l_returnValue;
    }

    // Close the root node (should have no effect, but let's follow best
    // practices!)
    l_returnValue = vfsOperationClose(l_root);

    if(l_returnValue != 0) {
        debug("kernel: Failed to close root node.\n");
        return l_returnValue;
    }

    return 0;
}
