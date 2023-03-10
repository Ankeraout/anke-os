#include <errno.h>
#include <string.h>

#include <kernel/arch/x86_64/inline.h>
#include <kernel/common.h>
#include <kernel/debug.h>
#include <kernel/module.h>
#include <kernel/fs/devfs.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <modules/ata.h>

#define C_ATA_STATUS_MASK_ERR 0x01
#define C_ATA_STATUS_MASK_IDX 0x02
#define C_ATA_STATUS_MASK_CORR 0x04
#define C_ATA_STATUS_MASK_DRQ 0x08
#define C_ATA_STATUS_MASK_SRV 0x10
#define C_ATA_STATUS_MASK_DF 0x20
#define C_ATA_STATUS_MASK_RDY 0x40
#define C_ATA_STATUS_MASK_BSY 0x80

enum {
    E_IOOFFSET_ATA_DATA = 0,
    E_IOOFFSET_ATA_ERROR = 1,
    E_IOOFFSET_ATA_FEATURES = 1,
    E_IOOFFSET_ATA_SECTOR_COUNT = 2,
    E_IOOFFSET_ATA_LBA_LOW = 3,
    E_IOOFFSET_ATA_SECTOR_NUMBER = 3,
    E_IOOFFSET_ATA_LBA_MID = 4,
    E_IOOFFSET_ATA_CYLINDER_LOW = 4,
    E_IOOFFSET_ATA_LBA_HIGH = 5,
    E_IOOFFSET_ATA_CYLINDER_HIGH = 5,
    E_IOOFFSET_ATA_DRIVE_HEAD = 6,
    E_IOOFFSET_ATA_STATUS = 7,
    E_IOOFFSET_ATA_COMMAND = 7
};

enum {
    E_ATA_COMMAND_READ_SECTORS = 0x20,
    E_ATA_COMMAND_READ_SECTORS_EXT = 0x24,
    E_ATA_COMMAND_IDENTIFY = 0xec
};

struct ts_ataDriverContext {
    int l_ataChannelCount;
};

struct ts_ataChannelContext;

struct ts_ataDriveContext {
    struct ts_ataChannelContext *a_channel;
    struct ts_vfsFileDescriptor *a_file;
    int a_driveNumber;
    bool a_usesLba;
};

struct ts_ataChannelContext {
    uint16_t a_ioPortBase;
    uint16_t a_ioPortControl;
    uint16_t a_ioPortBusMaster;
    int a_irq;
    struct ts_ataDriveContext a_drives[2];
    int a_selectedDrive;
    uint8_t a_registerCacheStatus;
    uint8_t a_registerCacheDriveHead;
};

static int ataInit(const char *p_args);
static void ataQuit(void);
static int ataIoctlDriver(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
);
static int ataIoctlCreate(
    struct ts_vfsFileDescriptor *p_driverFile,
    struct ts_ataRequestCreate *p_request
);
static int ataIoctlDeviceChannel(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
);
static void ataChannelScan(struct ts_vfsFileDescriptor *p_channelFile);
static void ataDriveScan(struct ts_ataDriveContext *p_context);
static void ataSelectDrive(struct ts_ataChannelContext *p_channel, int p_drive);
static void ataWait(struct ts_ataChannelContext *p_channel);
static void ataWaitUntilNotBusy(struct ts_ataChannelContext *p_channel);
static void ataWaitUntilDataRequestOrError(
    struct ts_ataChannelContext *p_channel
);

M_DECLARE_MODULE struct ts_module g_moduleAta = {
    .a_name = "ata",
    .a_init = ataInit,
    .a_quit = ataQuit
};

static int ataInit(const char *p_args) {
    M_UNUSED_PARAMETER(p_args);

    // Create ATA driver context
    struct ts_ataDriverContext *l_context =
        kmalloc(sizeof(struct ts_ataDriverContext));

    if(l_context == NULL) {
        debug("ata: Failed to allocate memory for driver context.\n");
        return -ENOMEM;
    }

    l_context->l_ataChannelCount = 0;

    // Create ATA driver file
    struct ts_vfsFileDescriptor *l_ataDriver =
        kcalloc(sizeof(struct ts_vfsFileDescriptor));

    if(l_ataDriver == NULL) {
        debug("ata: Failed to allocate memory for driver file.\n");
        kfree(l_context);
        return -ENOMEM;
    }

    strcpy(l_ataDriver->a_name, "ata");
    l_ataDriver->a_ioctl = ataIoctlDriver;
    l_ataDriver->a_type = E_VFS_FILETYPE_CHARACTER;
    l_ataDriver->a_context = l_context;

    // Register driver file
    struct ts_vfsFileDescriptor *l_devfs = vfsOpen("/dev", 0);

    if(l_devfs == NULL) {
        debug("ata: Failed to open /dev.\n");
        kfree(l_context);
        kfree(l_ataDriver);
        return -ENOENT;
    }

    int l_returnValue = l_devfs->a_ioctl(
        l_devfs,
        C_IOCTL_DEVFS_CREATE,
        l_ataDriver
    );

    kfree(l_devfs);

    if(l_returnValue != 0) {
        debug("ata: Failed to create driver file.\n");
        kfree(l_context);
        kfree(l_ataDriver);
        return l_returnValue;
    }

    debug("ata: Registered /dev/ata.\n");

    return 0;
}

static void ataQuit(void) {

}

static int ataIoctlDriver(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
) {
    M_UNUSED_PARAMETER(p_file);

    switch(p_request) {
        case E_IOCTL_ATA_DRIVER_CREATE: return ataIoctlCreate(p_file, p_arg);
        default: return -EINVAL;
    }

    return -EINVAL;
}

static int ataIoctlCreate(
    struct ts_vfsFileDescriptor *p_driverFile,
    struct ts_ataRequestCreate *p_request
) {
    debug(
        "ata: create(0x%04x, 0x%04x, 0x%04x, %d)\n",
        p_request->a_ioPortBase,
        p_request->a_ioPortControl,
        p_request->a_ioPortBusMaster,
        p_request->a_irq
    );

    struct ts_ataDriverContext *l_context = p_driverFile->a_context;

    // Create channel device file
    struct ts_vfsFileDescriptor *l_channelFile =
        kcalloc(sizeof(struct ts_vfsFileDescriptor));

    if(l_channelFile == NULL) {
        debug("ata: Failed to allocate memory for channel device file.\n");
        return -ENOMEM;
    }

    struct ts_ataChannelContext *l_channelContext =
        kcalloc(sizeof(struct ts_ataChannelContext));

    if(l_channelContext == NULL) {
        debug("ata: Failed to allocate memory for channel context.\n");
        kfree(l_channelFile);
        return -ENOMEM;
    }

    // Initialize channel context
    l_channelFile->a_context = l_channelContext;
    l_channelContext->a_ioPortBase = p_request->a_ioPortBase;
    l_channelContext->a_ioPortControl = p_request->a_ioPortControl;
    l_channelContext->a_ioPortBusMaster = p_request->a_ioPortBusMaster;
    l_channelContext->a_irq = p_request->a_irq;
    l_channelContext->a_drives[0].a_channel = l_channelContext;
    l_channelContext->a_drives[1].a_channel = l_channelContext;
    l_channelContext->a_drives[1].a_driveNumber = 1;

    strcpy(l_channelFile->a_name, "ata0");
    l_channelFile->a_name[3] += l_context->l_ataChannelCount++;
    l_channelFile->a_ioctl = ataIoctlDeviceChannel;
    l_channelFile->a_type = E_VFS_FILETYPE_CHARACTER;

    // Register channel device file
    struct ts_vfsFileDescriptor *l_devfs = vfsOpen("/dev", 0);

    if(l_devfs == NULL) {
        debug("ata: Failed to open /dev.\n");
        kfree(l_channelFile->a_context);
        kfree(l_channelFile);
        return 1;
    }

    int l_returnValue = l_devfs->a_ioctl(
        l_devfs,
        C_IOCTL_DEVFS_CREATE,
        l_channelFile
    );

    kfree(l_devfs);

    if(l_returnValue != 0) {
        debug("ata: Failed to register /dev/%s.\n", l_channelFile->a_name);
        kfree(l_channelFile->a_context);
        kfree(l_channelFile);
        return 1;
    }

    debug("ata: Registered channel as /dev/%s.\n", l_channelFile->a_name);

    // Scan the channel for drives
    l_channelFile->a_ioctl(
        l_channelFile,
        E_IOCTL_ATA_CHANNEL_SCAN,
        NULL
    );

    return 0;
}

static int ataIoctlDeviceChannel(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
) {
    M_UNUSED_PARAMETER(p_arg);

    switch(p_request) {
        case E_IOCTL_ATA_CHANNEL_SCAN:
            ataChannelScan(p_file);
            return 0;

        default:
            return -EINVAL;
    }

    return -EINVAL;
}

static void ataChannelScan(struct ts_vfsFileDescriptor *p_channelFile) {
    struct ts_ataChannelContext *l_context = p_channelFile->a_context;

    debug("ata: Scanning channel...\n");

    ataDriveScan(&l_context->a_drives[0]);
    ataDriveScan(&l_context->a_drives[1]);
}

static void ataDriveScan(struct ts_ataDriveContext *p_context) {
    debug("ata: Scanning drive %d...\n", p_context->a_driveNumber);

    ataSelectDrive(p_context->a_channel, p_context->a_driveNumber);

    outb(
        p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_SECTOR_COUNT,
        0x00
    );
    outb(
        p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_LOW,
        0x00
    );
    outb(
        p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_MID,
        0x00
    );
    outb(
        p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_HIGH,
        0x00
    );
    outb(
        p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_COMMAND,
        E_ATA_COMMAND_IDENTIFY
    );

    ataWait(p_context->a_channel);

    if(p_context->a_channel->a_registerCacheStatus == 0x00) {
        debug("ata: No drive detected.\n");
        return;
    }

    ataWaitUntilNotBusy(p_context->a_channel);

    // If LBA_MID or LBA_HIGH ports are non-zero, the drive is not an ATA drive.
    uint8_t l_lbaMid =
        inb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_MID);
    uint8_t l_lbaHigh =
        inb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_HIGH);
    uint16_t l_driveType = (l_lbaHigh << 8) | l_lbaMid;

    if(l_driveType == 0x0000) {
        debug("ata: Detected PATA drive.\n");
    } else if(l_driveType == 0xeb14) {
        debug("ata: Detected PATAPI drive.\n");
    } else if(l_driveType == 0x9669) {
        debug("ata: Detected SATA drive.\n");
    } else if(l_driveType == 0xc33c) {
        debug("ata: Detected SATAPI drive.\n");
    } else {
        debug("ata: Unknown drive type.\n");
    }
}

static void ataSelectDrive(
    struct ts_ataChannelContext *p_channel,
    int p_drive
) {
    if(p_channel->a_selectedDrive == p_drive) {
        return;
    }

    if(p_drive == 0) {
        p_channel->a_registerCacheDriveHead = 0xa0;
    } else {
        p_channel->a_registerCacheDriveHead = 0xb0;
    }

    if(p_channel->a_drives[p_drive].a_usesLba) {
        p_channel->a_registerCacheDriveHead |= 0x10;
    }

    outb(
        p_channel->a_ioPortBase + E_IOOFFSET_ATA_DRIVE_HEAD,
        p_channel->a_registerCacheDriveHead
    );

    ataWait(p_channel);

    p_channel->a_selectedDrive = p_drive;
}

static void ataWait(struct ts_ataChannelContext *p_channel) {
    for(int l_count = 0; l_count < 15; l_count++) {
        p_channel->a_registerCacheStatus =
            inb(p_channel->a_ioPortBase + E_IOOFFSET_ATA_STATUS);
    }
}

static void ataWaitUntilNotBusy(struct ts_ataChannelContext *p_channel) {
    do {
        p_channel->a_registerCacheStatus =
            inb(p_channel->a_ioPortBase + E_IOOFFSET_ATA_STATUS);
    } while((p_channel->a_registerCacheStatus & C_ATA_STATUS_MASK_BSY) != 0);
}

static void ataWaitUntilDataRequestOrError(
    struct ts_ataChannelContext *p_channel
) {
    const uint8_t l_mask = C_ATA_STATUS_MASK_DRQ | C_ATA_STATUS_MASK_ERR;

    do {
        p_channel->a_registerCacheStatus =
            inb(p_channel->a_ioPortBase + E_IOOFFSET_ATA_STATUS);
    } while((p_channel->a_registerCacheStatus & l_mask) == 0);
}
