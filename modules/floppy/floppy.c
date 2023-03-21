#include <errno.h>
#include <string.h>
#include <kernel/arch/x86_64/inline.h>
#include <kernel/common.h>
#include <kernel/debug.h>
#include <kernel/dev/device.h>
#include <kernel/module.h>
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
    struct ts_vfsNode *p_file,
    int p_request,
    void *p_arg
);
static ssize_t floppyRead(
    struct ts_vfsNode *p_file,
    off_t p_offset,
    void *p_buffer,
    size_t p_size
);
static int floppyCreate(const struct ts_floppyRequestCreate *p_request);

M_DECLARE_MODULE struct ts_module g_moduleFloppy = {
    .a_name = "floppy",
    .a_init = floppyInit,
    .a_quit = floppyQuit
};

static const uint16_t s_floppyControllerIoBase[] = {
    0x3f0,
    0x370,
    0x360
};
static dev_t s_floppyDeviceNumber;
static struct ts_floppyContextDevice s_floppyContextDevice[3];

static const struct ts_vfsNodeOperations s_floppyOperationsDriver = {
    .a_ioctl = floppyIoctl
};

static const struct ts_vfsNodeOperations s_floppyOperationsDevice = {
    .a_read = floppyRead
};

static int floppyInit(const char *p_args) {
    M_UNUSED_PARAMETER(p_args);

    // Create /dev/floppy file
    s_floppyDeviceNumber = deviceMake(0, 0);

    int l_returnValue =
        deviceRegister(E_DEVICETYPE_BLOCK, "fd", &s_floppyDeviceNumber, 4);

    if(l_returnValue != 0) {
        debug("floppy: Failed to register device.\n");
        return 1;
    }

    l_returnValue =
        deviceAdd("fd", s_floppyDeviceNumber, &s_floppyOperationsDriver, 1);

    if(l_returnValue != 0) {
        debug("floppy: Failed to add device.\n");
        return 1;
    }

    l_returnValue =
        deviceCreateFile2(s_floppyDeviceNumber, "floppy");

    if(l_returnValue != 0) {
        debug("floppy: Failed to create device file.\n");
        return 1;
    }

    debug("floppy: Registered /dev/floppy.\n");

    return 0;
}

static void floppyQuit(void) {

}

static int floppyIoctl(
    struct ts_vfsNode *p_file,
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
    struct ts_vfsNode *p_file,
    off_t p_offset,
    void *p_buffer,
    size_t p_size
) {
    M_UNUSED_PARAMETER(p_file);
    M_UNUSED_PARAMETER(p_offset);
    M_UNUSED_PARAMETER(p_buffer);
    M_UNUSED_PARAMETER(p_size);

    return 0;
}

static int floppyCreate(const struct ts_floppyRequestCreate *p_request) {
    s_floppyContextDevice[p_request->a_driveNumber].a_ioBase =
        s_floppyControllerIoBase[p_request->a_driveNumber];
    memcpy(
        &s_floppyContextDevice[p_request->a_driveNumber].a_geometry,
        &p_request->a_geometry,
        sizeof(struct ts_floppyGeometry)
    );

    // Create device file
    dev_t l_deviceNumber = s_floppyDeviceNumber;
    deviceSetMinor(&l_deviceNumber, p_request->a_driveNumber + 1);

    int l_returnValue =
        deviceAdd("fd", l_deviceNumber, &s_floppyOperationsDevice, 1);

    if(l_returnValue != 0) {
        debug("fd: Failed to add device.\n");
        return l_returnValue;
    }

    l_returnValue = deviceCreateFile(l_deviceNumber);

    if(l_returnValue != 0) {
        debug("fd: Failed to create device file.\n");
        return 1;
    }

    return 0;
}
