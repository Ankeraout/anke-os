#include <string.h>
#include <kernel/arch/x86_64/inline.h>
#include <kernel/common.h>
#include <kernel/debug.h>
#include <kernel/module.h>
#include <kernel/fs/devfs.h>
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
    struct ts_vfsFileDescriptor *p_file,
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

static struct ts_vfsFileDescriptor *s_rtcDevice;

static int cmosInit(const char *p_args) {
    M_UNUSED_PARAMETER(p_args);

    // Find the /dev node
    struct ts_vfsFileDescriptor *l_devfs = vfsFind("/dev");

    if(l_devfs == NULL) {
        debug("cmos: Failed to find /dev.\n");
        return 1;
    }

    // Create /dev/rtc file
    s_rtcDevice = kcalloc(sizeof(struct ts_vfsFileDescriptor));

    if(s_rtcDevice == NULL) {
        debug("cmos: Failed to allocate memory for RTC driver.\n");
        kfree(l_devfs);
        return 1;
    }

    strcpy(s_rtcDevice->a_name, "rtc");
    s_rtcDevice->a_type = E_VFS_FILETYPE_CHARACTER;
    s_rtcDevice->a_read = cmosRtcRead;

    // Register the RTC device file.
    if(
        l_devfs->a_ioctl(
            l_devfs,
            C_IOCTL_DEVFS_CREATE,
            s_rtcDevice
        ) != 0
    ) {
        debug("cmos: Failed to create /dev/rtc.\n");
        kfree(l_devfs);
        kfree(s_rtcDevice);
        return 1;
    }

    kfree(l_devfs);

    debug("cmos: Registered /dev/rtc.\n");

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
    struct ts_vfsFileDescriptor *p_file,
    void *p_buffer,
    size_t p_size
) {
    M_UNUSED_PARAMETER(p_file);
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
    struct ts_vfsFileDescriptor *l_floppyDriver = vfsFind("/dev/floppy");

    if(l_floppyDriver == NULL) {
        debug("cmos: Failed to open /dev/floppy.\n");
    }

    // Create floppy device
    struct ts_floppyRequestCreate l_requestFloppyCreate = {
        .a_geometry = l_floppyType->a_geometry,
        .a_driveNumber = p_driveNumber
    };

    l_floppyDriver->a_ioctl(
        l_floppyDriver,
        E_IOCTL_FLOPPY_CREATE,
        &l_requestFloppyCreate
    );
}
