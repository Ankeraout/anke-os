#include <errno.h>
#include <string.h>
#include <kernel/arch/x86_64/inline.h>
#include <kernel/common.h>
#include <kernel/debug.h>
#include <kernel/module.h>
#include <kernel/fs/devfs.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <modules/floppy.h>

struct ts_floppyContextDevice {
    uint16_t a_ioBase;
    struct ts_floppyGeometry a_geometry;
};

static int floppyInit(const char *p_args);
static void floppyQuit(void);
static int floppyIoctl(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
);
static ssize_t floppyRead(
    struct ts_vfsFileDescriptor *p_file,
    void *p_buffer,
    size_t p_size
);
static int floppyCreate(const struct ts_floppyRequestCreate *p_request);

M_DECLARE_MODULE struct ts_module g_moduleFloppy = {
    .a_name = "floppy",
    .a_init = floppyInit,
    .a_quit = floppyQuit
};

static struct ts_vfsFileDescriptor *s_floppyDriver;
static const uint16_t s_floppyControllerIoBase[] = {
    0x3f0,
    0x370,
    0x360
};

static int floppyInit(const char *p_args) {
    M_UNUSED_PARAMETER(p_args);

    // Find the /dev node
    struct ts_vfsFileDescriptor *l_devfs = vfsFind("/dev");

    if(l_devfs == NULL) {
        debug("floppy: Failed to find /dev.\n");
        return 1;
    }

    // Create /dev/floppy file
    s_floppyDriver = kcalloc(sizeof(struct ts_vfsFileDescriptor));

    if(s_floppyDriver == NULL) {
        debug("floppy: Failed to allocate memory for floppy driver.\n");
        kfree(l_devfs);
        return 1;
    }

    strcpy(s_floppyDriver->a_name, "floppy");
    s_floppyDriver->a_type = E_VFS_FILETYPE_CHARACTER;
    s_floppyDriver->a_ioctl = floppyIoctl;

    // Register the floppy driver file.
    if(
        l_devfs->a_ioctl(
            l_devfs,
            C_IOCTL_DEVFS_CREATE,
            s_floppyDriver
        ) != 0
    ) {
        debug("floppy: Failed to create /dev/floppy.\n");
        kfree(l_devfs);
        kfree(s_floppyDriver);
        return 1;
    }

    debug("floppy: Registered /dev/floppy.\n");

    return 0;
}

static void floppyQuit(void) {

}

static int floppyIoctl(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
) {
    M_UNUSED_PARAMETER(p_file);

    switch(p_request) {
        case E_IOCTL_FLOPPY_CREATE: return floppyCreate(p_arg);
        default:
            return -EINVAL;
    }

    return -EINVAL;
}

static ssize_t floppyRead(
    struct ts_vfsFileDescriptor *p_file,
    void *p_buffer,
    size_t p_size
) {
    M_UNUSED_PARAMETER(p_file);
    M_UNUSED_PARAMETER(p_buffer);
    M_UNUSED_PARAMETER(p_size);

    return 0;
}

static int floppyCreate(const struct ts_floppyRequestCreate *p_request) {
    // Allocate memory for the device context
    struct ts_floppyContextDevice *l_context =
        kcalloc(sizeof(struct ts_floppyContextDevice));

    if(l_context == NULL) {
        debug("floppy: Failed to allocate memory for device context.\n");
        return 1;
    }

    l_context->a_ioBase = s_floppyControllerIoBase[p_request->a_driveNumber];
    memcpy(
        &l_context->a_geometry,
        &p_request->a_geometry,
        sizeof(struct ts_floppyGeometry)
    );

    // Find the /dev node
    struct ts_vfsFileDescriptor *l_devfs = vfsFind("/dev");

    if(l_devfs == NULL) {
        debug("floppy: Failed to find /dev.\n");
        kfree(l_context);
        return 1;
    }

    // Create device file
    struct ts_vfsFileDescriptor *l_floppyDevice =
        kcalloc(sizeof(struct ts_vfsFileDescriptor));

    if(l_floppyDevice == NULL) {
        debug("floppy: Failed to allocate memory for floppy device.\n");
        kfree(l_context);
        kfree(l_devfs);
        return 1;
    }

    strcpy(l_floppyDevice->a_name, "fd");
    l_floppyDevice->a_name[2] = '0' + p_request->a_driveNumber;
    l_floppyDevice->a_name[3] = '\0';
    l_floppyDevice->a_type = E_VFS_FILETYPE_CHARACTER;
    l_floppyDevice->a_read = floppyRead;
    l_floppyDevice->a_context = l_context;

    // Register the floppy device file.
    if(
        l_devfs->a_ioctl(
            l_devfs,
            C_IOCTL_DEVFS_CREATE,
            l_floppyDevice
        ) != 0
    ) {
        debug("floppy: Failed to create /dev/%s.\n", l_floppyDevice->a_name);
        kfree(l_context);
        kfree(l_floppyDevice);
        kfree(l_devfs);
        return 1;
    }

    debug("floppy: Registered /dev/%s.\n", l_floppyDevice->a_name);

    return 0;
}
