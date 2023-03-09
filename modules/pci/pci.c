#include <errno.h>
#include <string.h>

#include <kernel/common.h>
#include <kernel/debug.h>
#include <kernel/module.h>
#include <kernel/fs/devfs.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>

static int pciInit(const char *p_arg);
static void pciQuit(void);
static int pciIoctlDriver(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
);

M_DECLARE_MODULE struct ts_module g_modulePci = {
    .a_name = "pci",
    .a_init = pciInit,
    .a_quit = pciQuit
};

static int pciInit(const char *p_arg) {
    M_UNUSED_PARAMETER(p_arg);

    debug("pci: Initializing module...\n");

    // Create PCI driver file
    struct ts_vfsFileDescriptor *l_pciDriver =
        kcalloc(sizeof(struct ts_vfsFileDescriptor));

    if(l_pciDriver == NULL) {
        debug("pci: Failed to allocate memory for driver file.\n");
        return -ENOMEM;
    }

    strcpy(l_pciDriver->a_name, "pci");
    l_pciDriver->a_ioctl = pciIoctlDriver;
    l_pciDriver->a_type = E_VFS_FILETYPE_CHARACTER;

    // Register driver file
    struct ts_vfsFileDescriptor *l_dev = vfsOpen("/dev", 0);

    if(l_dev == NULL) {
        debug("pci: Failed to open /dev.\n");
        kfree(l_pciDriver);
        return -ENOENT;
    }

    int l_returnValue = l_dev->a_ioctl(
        l_dev,
        C_IOCTL_DEVFS_CREATE,
        l_pciDriver
    );

    kfree(l_dev);

    if(l_returnValue != 0) {
        debug("pci: Failed to create driver file.\n");
        kfree(l_pciDriver);
        return l_returnValue;
    }

    debug("pci: Registered /dev/pci.\n");

    debug("pci: Module initialized successfully.\n");

    return 0;
}

static void pciQuit(void) {

}

static int pciIoctlDriver(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
) {
    M_UNUSED_PARAMETER(p_file);
    M_UNUSED_PARAMETER(p_request);
    M_UNUSED_PARAMETER(p_arg);

    return -EINVAL;
}
