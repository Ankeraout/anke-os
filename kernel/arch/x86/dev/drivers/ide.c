#include <string.h>

#include <kernel/arch/x86/dev/drivers/ide.h>
#include <kernel/arch/x86/inline.h>
#include <kernel/dev/device.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/common.h>
#include <kernel/debug.h>

#define C_IDE_STATUS_MASK_ERR 0x01
#define C_IDE_STATUS_MASK_IDX 0x02
#define C_IDE_STATUS_MASK_CORR 0x04
#define C_IDE_STATUS_MASK_DRQ 0x08
#define C_IDE_STATUS_MASK_SRV 0x10
#define C_IDE_STATUS_MASK_DF 0x20
#define C_IDE_STATUS_MASK_RDY 0x40
#define C_IDE_STATUS_MASK_BSY 0x80

enum te_devIdeIoPortOffset {
    E_IOOFFSET_IDE_DATA = 0,
    E_IOOFFSET_IDE_ERROR = 1,
    E_IOOFFSET_IDE_FEATURES = 1,
    E_IOOFFSET_IDE_SECTOR_COUNT = 2,
    E_IOOFFSET_IDE_LBA_LOW = 3,
    E_IOOFFSET_IDE_SECTOR_NUMBER = 3,
    E_IOOFFSET_IDE_LBA_MID = 4,
    E_IOOFFSET_IDE_CYLINDER_LOW = 4,
    E_IOOFFSET_IDE_LBA_HIGH = 5,
    E_IOOFFSET_IDE_CYLINDER_HIGH = 5,
    E_IOOFFSET_IDE_DRIVE_HEAD = 6,
    E_IOOFFSET_IDE_STATUS = 7,
    E_IOOFFSET_IDE_COMMAND = 7
};

enum te_devIdeCommand {
    E_IDE_COMMAND_READ_SECTORS = 0x20,
    E_IDE_COMMAND_READ_SECTORS_EXT = 0x24,
    E_IDE_COMMAND_IDENTIFY = 0xec
};

enum te_devIdeDrive {
    E_DEV_IDE_DRIVE_MASTER,
    E_DEV_IDE_DRIVE_SLAVE
};

struct ts_deviceDriverDataIde2 {
    struct ts_deviceDriverDataIde a_data;
    uint8_t a_registerCacheStatus;
    uint8_t a_registerCacheDriveHead;
    enum te_devIdeDrive a_selectedDrive;
};

static int ideInit(struct ts_device *p_device);
static int ideDetectDrive(
    struct ts_device *p_device,
    enum te_devIdeDrive p_drive
);
static void ideSelectDrive(
    struct ts_device *p_device,
    enum te_devIdeDrive p_drive
);
static void ideWait(struct ts_device *p_device);
static void ideWaitBusy(struct ts_device *p_device);
static void ideWaitDataRequestOrError(struct ts_device *p_device);
static size_t ideDriverApiGetChildCount(struct ts_device *p_device);
static struct ts_device *ideDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
);

const struct ts_deviceDriver g_deviceDriverIde = {
    .a_name = "IDE channel controller",
    .a_api = {
        .a_init = ideInit,
        .a_getChild = ideDriverApiGetChild,
        .a_getChildCount = ideDriverApiGetChildCount,
        .a_isSupported = NULL
    }
};

static int ideInit(struct ts_device *p_device) {
    const struct ts_deviceDriverDataIde *l_driverParameters =
        (const struct ts_deviceDriverDataIde *)p_device->a_driverData;

    p_device->a_driverData = kmalloc(sizeof(struct ts_deviceDriverDataIde2));

    if(p_device->a_driverData == NULL) {
        debug("ide: Failed to allocate memory for driver data.\n");
        return 1;
    }

    memcpy(
        p_device->a_driverData,
        l_driverParameters,
        sizeof(struct ts_deviceDriverDataIde)
    );

    debug(
        "ide: Initializing IDE channel (0x%04x, 0x%04x, 0x%04x, %d)...\n",
        l_driverParameters->a_ioBase,
        l_driverParameters->a_ioControl,
        l_driverParameters->a_ioBusMaster,
        l_driverParameters->a_irq
    );

    ideDetectDrive(p_device, E_DEV_IDE_DRIVE_MASTER);
    ideDetectDrive(p_device, E_DEV_IDE_DRIVE_SLAVE);

    return 0;
}

static int ideDetectDrive(
    struct ts_device *p_device,
    enum te_devIdeDrive p_drive
) {
    struct ts_deviceDriverDataIde2 *l_deviceDriverData =
        (struct ts_deviceDriverDataIde2 *)p_device->a_driverData;

    ideSelectDrive(p_device, p_drive);

    debug("ide: Checking for ");
    debug(p_drive == E_DEV_IDE_DRIVE_MASTER ? "master" : "slave");
    debug(" drive...\n");

    outb(
        l_deviceDriverData->a_data.a_ioBase + E_IOOFFSET_IDE_SECTOR_COUNT,
        0x00
    );
    outb(
        l_deviceDriverData->a_data.a_ioBase + E_IOOFFSET_IDE_LBA_LOW,
        0x00
    );
    outb(
        l_deviceDriverData->a_data.a_ioBase + E_IOOFFSET_IDE_LBA_MID,
        0x00
    );
    outb(
        l_deviceDriverData->a_data.a_ioBase + E_IOOFFSET_IDE_LBA_HIGH,
        0x00
    );
    outb(
        l_deviceDriverData->a_data.a_ioBase + E_IOOFFSET_IDE_COMMAND,
        E_IDE_COMMAND_IDENTIFY
    );

    ideWait(p_device);

    if(inb(l_deviceDriverData->a_data.a_ioBase + E_IOOFFSET_IDE_STATUS) == 0x00) {
        debug("ide: No drive detected.\n");
        return 1;
    }

    ideWaitBusy(p_device);

    // If LBA_MID or LBA_HIGH ports are non-zero, the drive is not an ATA drive.
    uint8_t l_lbaMid = inb(l_deviceDriverData->a_data.a_ioBase + E_IOOFFSET_IDE_LBA_MID);
    uint8_t l_lbaHigh = inb(l_deviceDriverData->a_data.a_ioBase + E_IOOFFSET_IDE_LBA_HIGH);
    uint16_t l_driveType = (l_lbaHigh << 8) | l_lbaMid;

    if(l_driveType == 0) {
        debug("ide: Detected PATA drive.\n");
    } else if(l_driveType == 0xeb14) {
        debug("ide: Detected PATAPI drive.\n");
    } else if(l_driveType == 0x9669) {
        debug("ide: Detected SATAPI drive.\n");
    } else if(l_driveType == 0xc33c) {
        debug("ide: Detected SATA drive.\n");
    } else {
        debug("ide: Unknown drive type.\n");
        return 1;
    }

    return 0;
}

static void ideSelectDrive(
    struct ts_device *p_device,
    enum te_devIdeDrive p_drive
) {
    struct ts_deviceDriverDataIde2 *l_deviceDriverData =
        (struct ts_deviceDriverDataIde2 *)p_device->a_driverData;

    if(l_deviceDriverData->a_selectedDrive == p_drive) {
        return;
    }

    l_deviceDriverData->a_registerCacheDriveHead = 0xa0;

    if(p_drive == E_DEV_IDE_DRIVE_SLAVE) {
        l_deviceDriverData->a_registerCacheDriveHead |= 0x10;
    }

    // TODO: OR with 0x40 if drive uses LBA

    outb(
        l_deviceDriverData->a_data.a_ioBase + E_IOOFFSET_IDE_DRIVE_HEAD,
        l_deviceDriverData->a_registerCacheDriveHead
    );

    ideWait(p_device);

    l_deviceDriverData->a_selectedDrive = p_drive;
}

static void ideWait(struct ts_device *p_device) {
    const struct ts_deviceDriverDataIde *l_deviceDriverData =
        (const struct ts_deviceDriverDataIde *)p_device->a_driverData;

    for(int l_count = 0; l_count < 15; l_count++) {
        inb(l_deviceDriverData->a_ioBase + E_IOOFFSET_IDE_STATUS);
    }
}

static void ideWaitBusy(struct ts_device *p_device) {
    struct ts_deviceDriverDataIde2 *l_deviceDriverData =
        (struct ts_deviceDriverDataIde2 *)p_device->a_driverData;

    do {
        l_deviceDriverData->a_registerCacheStatus = inb(
            l_deviceDriverData->a_data.a_ioBase + E_IOOFFSET_IDE_STATUS
        );
    } while(
        (
            l_deviceDriverData->a_registerCacheStatus
                & C_IDE_STATUS_MASK_BSY
        ) != 0);
}

static void ideWaitDataRequestOrError(struct ts_device *p_device) {
    struct ts_deviceDriverDataIde2 *l_deviceDriverData =
        (struct ts_deviceDriverDataIde2 *)p_device->a_driverData;

    const uint8_t l_mask = C_IDE_STATUS_MASK_DRQ | C_IDE_STATUS_MASK_ERR;

    do {
        l_deviceDriverData->a_registerCacheStatus = inb(
            l_deviceDriverData->a_data.a_ioBase + E_IOOFFSET_IDE_STATUS
        );
    } while((l_deviceDriverData->a_registerCacheStatus & l_mask) == 0);
}

static size_t ideDriverApiGetChildCount(struct ts_device *p_device) {
    M_UNUSED_PARAMETER(p_device);

    return 0;
}

static struct ts_device *ideDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
) {
    M_UNUSED_PARAMETER(p_device);
    M_UNUSED_PARAMETER(p_index);

    return NULL;
}
