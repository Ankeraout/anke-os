#include <string.h>
#include <kernel/arch/x86_64/inline.h>
#include <kernel/common.h>
#include <kernel/debug.h>
#include <kernel/dev/device.h>
#include <kernel/module.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <modules/floppy.h>

#define C_IOPORT_CMOS_REGISTER_NUMBER 0x70
#define C_IOPORT_CMOS_DATA 0x71

enum {
    E_CMOS_REGISTER_FLOPPY_INFO = 0x10
};

enum te_cmosFloppyType {
    E_CMOS_FLOPPYTYPE_NONE,
    E_CMOS_FLOPPYTYPE_5_25_360KB,
    E_CMOS_FLOPPYTYPE_5_25_1200KB,
    E_CMOS_FLOPPYTYPE_3_5_720KB,
    E_CMOS_FLOPPYTYPE_3_5_1440KB,
    E_CMOS_FLOPPYTYPE_3_5_2880KB
};

struct ts_cmosFloppyType {
    struct ts_floppyGeometry a_geometry;
    const char *a_name;
};

static const struct ts_cmosFloppyType s_cmosFloppyTypes[] = {
    {.a_name = "None"}, // No drive
    {
        .a_geometry = {.a_cylinders = 40, .a_sectors = 9},
        .a_name = "5.25\" 360KB"
    },
    {
        .a_geometry = {.a_cylinders = 80, .a_sectors = 15},
        .a_name = "5.25\" 1.2MB"
    },
    {
        .a_geometry = {.a_cylinders = 80, .a_sectors = 9},
        .a_name = "3.5\" 720KB"
    },
    {
        .a_geometry = {.a_cylinders = 80, .a_sectors = 18},
        .a_name = "3.5\" 1.44MB"
    },
    {
        .a_geometry = {.a_cylinders = 80, .a_sectors = 36},
        .a_name = "3.5\" 2.88MB"
    },
};

static int cmosInit(const char *p_args);
static void cmosQuit(void);
static ssize_t cmosRtcRead(
    struct ts_vfsNode *p_file,
    off_t p_offset,
    void *p_buffer,
    size_t p_size
);
static void cmosCheckFloppy(
    int p_driveNumber,
    enum te_cmosFloppyType p_type
);

M_DECLARE_MODULE struct ts_module g_moduleCmos = {
    .a_name = "cmos",
    .a_init = cmosInit,
    .a_quit = cmosQuit
};

static dev_t s_cmosDeviceNumber;
static const struct ts_vfsNodeOperations s_rtcOperations = {
    .a_read = cmosRtcRead
};

static int cmosInit(const char *p_args) {
    M_UNUSED_PARAMETER(p_args);

    // Create /dev/rtc file
    s_cmosDeviceNumber = deviceMake(0, 0);

    int l_returnValue =
        deviceRegister(E_DEVICETYPE_CHARACTER, "cmos", &s_cmosDeviceNumber, 1);

    if(l_returnValue != 0) {
        debug("cmos: Failed to register CMOS device driver.\n");
        return 1;
    }

    l_returnValue =
        deviceAdd("cmos", s_cmosDeviceNumber, &s_rtcOperations, 1);

    if(l_returnValue != 0) {
        debug("cmos: Failed to add RTC device.\n");
        return 1;
    }

    l_returnValue =
        deviceCreateFile2(s_cmosDeviceNumber, "cmos");

    if(l_returnValue != 0) {
        debug("cmos: Failed to create RTC device file.\n");
        return 1;
    }

    // Read floppy disk information (register 0x10)
    outb(C_IOPORT_CMOS_REGISTER_NUMBER, E_CMOS_REGISTER_FLOPPY_INFO);
    uint8_t l_floppyInfo = inb(C_IOPORT_CMOS_DATA);

    cmosCheckFloppy(0, l_floppyInfo >> 4);
    cmosCheckFloppy(1, l_floppyInfo & 0x0f);

    return 0;
}

static void cmosQuit(void) {

}

static ssize_t cmosRtcRead(
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

static void cmosCheckFloppy(
    int p_driveNumber,
    enum te_cmosFloppyType p_type
) {
    // If the drive type is not supported, return
    if(
        (p_type == E_CMOS_FLOPPYTYPE_NONE)
        || (p_type > 5)
    ) {
        debug("cmos: Detected no floppy driver as drive %d.\n", p_driveNumber);
        return;
    }

    const struct ts_cmosFloppyType *l_floppyType = &s_cmosFloppyTypes[p_type];

    debug(
        "cmos: Detected %s floppy driver as drive %d.\n",
        l_floppyType->a_name,
        p_driveNumber
    );

    // Make sure that the floppy module is loaded
    const struct ts_module *l_floppyModule = moduleGetKernelModule("floppy");

    if(l_floppyModule == NULL) {
        debug("cmos: floppy module was not found.\n");
        return;
    }

    if(!moduleIsModuleLoaded(l_floppyModule)) {
        if(moduleLoad(l_floppyModule, NULL) != 0) {
            debug("cmos: Failed to load floppy module.\n");
            return;
        }
    }

    // Find /dev/floppy
    struct ts_vfsNode *l_floppyDriver;
    int l_returnValue = vfsLookup(NULL, "/dev/floppy", &l_floppyDriver);

    if(l_returnValue != 0) {
        debug("cmos: Failed to open /dev/floppy: %d\n", l_returnValue);
        return;
    }

    // Create floppy device
    struct ts_floppyRequestCreate l_requestFloppyCreate = {
        .a_geometry = l_floppyType->a_geometry,
        .a_driveNumber = p_driveNumber
    };

    vfsOperationIoctl(
        l_floppyDriver,
        E_IOCTL_FLOPPY_CREATE,
        &l_requestFloppyCreate
    );

    vfsOperationClose(l_floppyDriver);
}
