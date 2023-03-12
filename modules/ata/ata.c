#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include <kernel/arch/x86_64/inline.h>
#include <kernel/common.h>
#include <kernel/debug.h>
#include <kernel/device.h>
#include <kernel/module.h>
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

struct ts_ataChannelContext;

struct ts_ataDriveContext {
    struct ts_ataChannelContext *a_channel;
    struct ts_vfsFileDescriptor *a_file;
    int a_driveNumber;
    bool a_usesLba;
    bool a_usesLba48;
    uint64_t a_sectorCount;
    uint16_t a_cylinders;
    uint8_t a_heads;
    uint8_t a_sectors;
    off_t a_position;
};

struct ts_ataChannelContext {
    struct ts_vfsFileDescriptor *a_file;
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
static int ataOpen(struct ts_vfsFileDescriptor *p_file, int p_flags);
static off_t ataLseek(
    struct ts_vfsFileDescriptor *p_file,
    off_t p_offset,
    int p_whence
);
static ssize_t ataReadPata(
    struct ts_vfsFileDescriptor *p_file,
    void *p_buffer,
    size_t p_size
);
static ssize_t ataWritePata(
    struct ts_vfsFileDescriptor *p_file,
    const void *p_buffer,
    size_t p_size
);
static ssize_t ataReadPataSector(
    struct ts_ataDriveContext *p_context,
    uint64_t p_lba,
    void *p_buffer
);

M_DECLARE_MODULE struct ts_module g_moduleAta = {
    .a_name = "ata",
    .a_init = ataInit,
    .a_quit = ataQuit
};

static int ataInit(const char *p_args) {
    M_UNUSED_PARAMETER(p_args);

    // Create ATA driver file
    struct ts_vfsFileDescriptor *l_ataDriver =
        kcalloc(sizeof(struct ts_vfsFileDescriptor));

    if(l_ataDriver == NULL) {
        debug("ata: Failed to allocate memory for driver file.\n");
        return 1;
    }

    strcpy(l_ataDriver->a_name, "ata");
    l_ataDriver->a_ioctl = ataIoctlDriver;
    l_ataDriver->a_type = E_VFS_FILETYPE_CHARACTER;

    // Register driver file
    if(deviceMount("/dev/ata", l_ataDriver) != 0) {
        debug("ata: Failed to create driver file.\n");
        kfree(l_ataDriver);
        return 1;
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
    M_UNUSED_PARAMETER(p_driverFile);

    // Create channel device file
    struct ts_vfsFileDescriptor *l_channelFile =
        deviceCreate("/dev/ata%d", 0);

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
    l_channelFile->a_ioctl = ataIoctlDeviceChannel;
    l_channelFile->a_type = E_VFS_FILETYPE_CHARACTER;

    l_channelContext->a_ioPortBase = p_request->a_ioPortBase;
    l_channelContext->a_ioPortControl = p_request->a_ioPortControl;
    l_channelContext->a_ioPortBusMaster = p_request->a_ioPortBusMaster;
    l_channelContext->a_irq = p_request->a_irq;
    l_channelContext->a_drives[0].a_channel = l_channelContext;
    l_channelContext->a_drives[1].a_channel = l_channelContext;
    l_channelContext->a_drives[1].a_driveNumber = 1;
    l_channelContext->a_file = l_channelFile;

    // Register channel device file
    char l_path[PATH_MAX];

    snprintf(l_path, PATH_MAX, "/dev/%s", l_channelFile->a_name);

    if(vfsMount(l_path, l_channelFile) != 0) {
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

    struct ts_vfsFileDescriptor *l_driveFile = NULL;

    if(l_driveType == 0x0000) {
        debug("ata: Detected PATA drive.\n");

        ataWaitUntilDataRequestOrError(p_context->a_channel);

        // Read identification data
        uint16_t l_identificationData[256];

        for(int l_index = 0; l_index < 256; l_index++) {
            l_identificationData[l_index] =
                inw(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_DATA);
        }

        debug("ata: Drive name: ");

        for(int l_index = 0; l_index < 20; l_index++) {
            const uint16_t l_word = l_identificationData[27 + l_index];
            debug("%c%c", l_word >> 8, l_word);
        }

        debug("\n");

        const uint64_t l_totalSectorsLong =
            (uint64_t)l_identificationData[100]
            | ((uint64_t)l_identificationData[101] << 16)
            | ((uint64_t)l_identificationData[102] << 32)
            | ((uint64_t)l_identificationData[103] << 48);
        const uint32_t l_totalSectors =
            (uint32_t)l_identificationData[60]
            | ((uint32_t)l_identificationData[61] << 16);

        if(l_totalSectorsLong != 0) {
            debug("ata: Drive supports LBA48.\n");
            p_context->a_usesLba = true;
            p_context->a_usesLba48 = true;
            p_context->a_sectorCount = l_totalSectorsLong;
        } else if(l_totalSectors != 0) {
            debug("ata: Drive supports LBA.\n");
            p_context->a_usesLba = true;
            p_context->a_sectorCount = l_totalSectors;

            if((l_identificationData[83] & (1 << 10)) != 0) {
                debug("ata: Drive supports LBA48.\n");
                p_context->a_usesLba48 = true;
            } else {
                debug("ata: Drive supports LBA28.\n");
                p_context->a_usesLba48 = false;
            }
        } else {
            debug("ata: Drive does not support LBA.\n");
            p_context->a_usesLba = false;
            p_context->a_usesLba48 = false;
            p_context->a_cylinders = l_identificationData[1];
            p_context->a_heads = l_identificationData[3];
            p_context->a_sectors = l_identificationData[6];
            p_context->a_sectorCount =
                p_context->a_cylinders
                * p_context->a_heads
                * p_context->a_sectors;
        }

        // Create device file
        l_driveFile = kcalloc(sizeof(struct ts_vfsFileDescriptor));

        if(l_driveFile == NULL) {
            debug("ata: Failed to allocate memory for drive file.\n");
            return;
        }

        // Assign drive file operations
        l_driveFile->a_read = ataReadPata;
        l_driveFile->a_write = ataWritePata;
    } else if(l_driveType == 0xeb14) {
        debug("ata: Detected PATAPI drive.\n");
    } else if(l_driveType == 0x9669) {
        debug("ata: Detected SATA drive.\n");
    } else if(l_driveType == 0xc33c) {
        debug("ata: Detected SATAPI drive.\n");
    } else {
        debug("ata: Unknown drive type.\n");
    }

    if(l_driveFile != NULL) {
        strcpy(l_driveFile->a_name, p_context->a_channel->a_file->a_name);
        l_driveFile->a_name[4] = '-';
        l_driveFile->a_name[5] = '0' + p_context->a_driveNumber;
        l_driveFile->a_name[6] = '\0';
        l_driveFile->a_type = E_VFS_FILETYPE_BLOCK;
        l_driveFile->a_open = ataOpen;
        l_driveFile->a_lseek = ataLseek;
        l_driveFile->a_context = p_context;

        if(deviceMount("/dev/%s", l_driveFile) != 0) {
            debug("ata: Failed to create /dev/%s.\n", l_driveFile->a_name);
            kfree(l_driveFile);
            return;
        }

        debug("ata: Registered /dev/%s.\n", l_driveFile->a_name);
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
        p_channel->a_registerCacheDriveHead |= 0x40;
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

static int ataOpen(struct ts_vfsFileDescriptor *p_file, int p_flags) {
    M_UNUSED_PARAMETER(p_flags);

    struct ts_ataDriveContext *l_context = p_file->a_context;

    l_context->a_position = 0;

    return 0;
}

static off_t ataLseek(
    struct ts_vfsFileDescriptor *p_file,
    off_t p_offset,
    int p_whence
) {
    struct ts_ataDriveContext *l_context = p_file->a_context;

    if(p_whence == SEEK_CUR) {
        l_context->a_position += p_offset;
    } else if(p_whence == SEEK_END) {
        l_context->a_position = l_context->a_sectorCount * 512 + p_offset;
    } else if(p_whence == SEEK_SET) {
        l_context->a_position = p_offset;
    } else {
        return -1;
    }

    return l_context->a_position;
}

static ssize_t ataReadPata(
    struct ts_vfsFileDescriptor *p_file,
    void *p_buffer,
    size_t p_size
) {
    struct ts_ataDriveContext *l_context = p_file->a_context;

    // Compute offset
    size_t l_sector = l_context->a_position >> 9;
    size_t l_sectorOffset = l_context->a_position & 0x1ff;
    size_t l_bytesRead = 0;

    while(l_bytesRead < p_size) {
        uint8_t l_buffer[512];

        if(ataReadPataSector(l_context, l_sector, l_buffer) != 512) {
            return (ssize_t)l_bytesRead;
        }

        while((l_sectorOffset < 512) && (l_bytesRead < p_size)) {
            ((uint8_t *)p_buffer)[l_bytesRead++] = l_buffer[l_sectorOffset++];
        }

        l_sectorOffset = 0;
    }

    return (ssize_t)l_bytesRead;
}

static ssize_t ataWritePata(
    struct ts_vfsFileDescriptor *p_file,
    const void *p_buffer,
    size_t p_size
) {
    M_UNUSED_PARAMETER(p_file);
    M_UNUSED_PARAMETER(p_buffer);
    M_UNUSED_PARAMETER(p_size);

    return 0;
}

static ssize_t ataReadPataSector(
    struct ts_ataDriveContext *p_context,
    uint64_t p_lba,
    void *p_buffer
) {
    // Cannot read beyond drive capacity.
    if(p_lba >= p_context->a_sectorCount) {
        return 0;
    }

    ataSelectDrive(p_context->a_channel, p_context->a_driveNumber);
    ataWaitUntilNotBusy(p_context->a_channel);

    // Set register values
    p_context->a_channel->a_registerCacheDriveHead &= 0xf0;

    if(p_context->a_usesLba) {
        if(p_context->a_usesLba48) {
            outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_DRIVE_HEAD, p_context->a_channel->a_registerCacheDriveHead);
            outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_SECTOR_COUNT, 0);
            outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_LOW, p_lba >> 24);
            outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_MID, p_lba >> 32);
            outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_HIGH, p_lba >> 40);
            outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_SECTOR_COUNT, 1);
            outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_LOW, p_lba);
            outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_MID, p_lba >> 8);
            outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_HIGH, p_lba >> 16);
            outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_COMMAND, E_ATA_COMMAND_READ_SECTORS_EXT);
        } else {
            p_context->a_channel->a_registerCacheDriveHead |= (p_lba >> 24) & 0x0f;
            outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_DRIVE_HEAD, p_context->a_channel->a_registerCacheDriveHead);
            ataWait(p_context->a_channel);

            outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_SECTOR_COUNT, 1);
            outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_LOW, p_lba);
            outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_MID, p_lba >> 8);
            outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_HIGH, p_lba >> 16);
            outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_COMMAND, E_ATA_COMMAND_READ_SECTORS);
        }
    } else {
        uint32_t l_sectorsPerCylinder = p_context->a_sectors * p_context->a_heads;
        uint8_t l_head = (p_lba % l_sectorsPerCylinder) / p_context->a_sectors;
        uint16_t l_cylinder = p_lba / l_sectorsPerCylinder;
        uint8_t l_sector = p_lba % p_context->a_sectors + 1;

        p_context->a_channel->a_registerCacheDriveHead |= l_head & 0x0f;
        outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_DRIVE_HEAD, p_context->a_channel->a_registerCacheDriveHead);
        ataWait(p_context->a_channel);

        outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_SECTOR_COUNT, 1);
        outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_SECTOR_NUMBER, l_sector);
        outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_CYLINDER_LOW, l_cylinder);
        outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_CYLINDER_HIGH, l_cylinder >> 8);
        outb(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_COMMAND, E_ATA_COMMAND_READ_SECTORS);
    }

    ataWaitUntilNotBusy(p_context->a_channel);
    ataWaitUntilDataRequestOrError(p_context->a_channel);

    if((p_context->a_channel->a_registerCacheDriveHead & C_ATA_STATUS_MASK_ERR) != 0) {
        return 0;
    }

    for(size_t l_bufferIndex = 0; l_bufferIndex < 256; l_bufferIndex++) {
        ((uint16_t *)p_buffer)[l_bufferIndex] = inw(p_context->a_channel->a_ioPortBase + E_IOOFFSET_ATA_DATA);
    }

    return 512;
}
