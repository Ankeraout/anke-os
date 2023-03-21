#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <kernel/arch/x86_64/inline.h>
#include <kernel/common.h>
#include <kernel/debug.h>
#include <kernel/dev/device.h>
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

struct ts_ataDrive {
    bool a_usesLba;
    bool a_usesLba48;
    uint64_t a_sectorCount;
    uint16_t a_cylinders;
    uint8_t a_heads;
    uint8_t a_sectors;
};

struct ts_ataChannel {
    uint16_t a_ioPortBase;
    uint16_t a_ioPortControl;
    uint16_t a_ioPortBusMaster;
    int a_irq;
    struct ts_ataDrive a_drives[2];
    int a_selectedDrive;
    uint8_t a_registerCacheStatus;
    uint8_t a_registerCacheDriveHead;
};

static int ataInit(const char *p_args);
static void ataQuit(void);
static int ataIoctlDriver(
    struct ts_vfsNode *p_file,
    int p_request,
    void *p_arg
);
static int ataIoctlDriverCreate(
    struct ts_vfsNode *p_driverFile,
    struct ts_ataRequestCreate *p_request
);
static int ataIoctlChannel(
    struct ts_vfsNode *p_file,
    int p_request,
    void *p_arg
);
static void ataIoctlChannelScan(struct ts_vfsNode *p_channelFile);
static void ataDriveScan(int p_channelNumber, int p_driveNumber);
static void ataSelectDrive(int p_channelNumber, int p_driveNumber);
static void ataWait(int p_channelNumber);
static void ataWaitUntilNotBusy(int p_channelNumber);
static void ataWaitUntilDataRequestOrError(int p_channelNumber);
static int ataOpen(struct ts_vfsNode *p_file, int p_flags);
static ssize_t ataReadPata(
    struct ts_vfsNode *p_file,
    off_t p_offset,
    void *p_buffer,
    size_t p_size
);
static ssize_t ataWritePata(
    struct ts_vfsNode *p_file,
    off_t p_offset,
    const void *p_buffer,
    size_t p_size
);
static ssize_t ataReadPataSector(
    int p_channelNumber,
    int p_driveNumber,
    uint64_t p_lba,
    void *p_buffer
);

M_DECLARE_MODULE struct ts_module g_moduleAta = {
    .a_name = "ata",
    .a_init = ataInit,
    .a_quit = ataQuit
};

static const struct ts_vfsNodeOperations s_ataOperationsDriver = {
    .a_ioctl = ataIoctlDriver
};

static const struct ts_vfsNodeOperations s_ataOperationsChannel = {
    .a_ioctl = ataIoctlChannel
};

static const struct ts_vfsNodeOperations s_ataOperationsDrivePata = {
    .a_open = ataOpen,
    .a_read = ataReadPata,
    .a_write = ataWritePata,
};

static dev_t s_ataDeviceNumber;
static int s_ataChannelCount;
static struct ts_ataChannel s_ataChannelContext[2];

static int ataInit(const char *p_args) {
    M_UNUSED_PARAMETER(p_args);

    s_ataChannelCount = 0;

    // Create ATA driver file
    s_ataDeviceNumber = deviceMake(0, 0);

    int l_returnValue =
        deviceRegister(E_DEVICETYPE_BLOCK, "ata", &s_ataDeviceNumber, 7);

    if(l_returnValue != 0) {
        debug("ata: Failed to register device: %d.\n", l_returnValue);
        return 1;
    }

    l_returnValue =
        deviceAdd("ata", s_ataDeviceNumber, &s_ataOperationsDriver, 1);

    if(l_returnValue != 0) {
        debug("ata: Failed to add device: %d.\n", l_returnValue);
        return 1;
    }

    l_returnValue =
        deviceCreateFile2(s_ataDeviceNumber, "ata");

    if(l_returnValue != 0) {
        debug("ata: Failed to create device file: %d.\n", l_returnValue);
        return 1;
    }

    debug("ata: Registered /dev/ata.\n");

    return 0;
}

static void ataQuit(void) {

}

static int ataIoctlDriver(
    struct ts_vfsNode *p_file,
    int p_request,
    void *p_arg
) {
    M_UNUSED_PARAMETER(p_file);

    switch(p_request) {
        case E_IOCTL_ATA_DRIVER_CREATE:
            return ataIoctlDriverCreate(p_file, p_arg);

        default: return -EINVAL;
    }

    return -EINVAL;
}

static int ataIoctlDriverCreate(
    struct ts_vfsNode *p_driverFile,
    struct ts_ataRequestCreate *p_request
) {
    M_UNUSED_PARAMETER(p_driverFile);

    int l_channelIndex = s_ataChannelCount;

    // Initialize channel context
    s_ataChannelContext[l_channelIndex].a_ioPortBase =
        p_request->a_ioPortBase;
    s_ataChannelContext[l_channelIndex].a_ioPortControl =
        p_request->a_ioPortControl;
    s_ataChannelContext[l_channelIndex].a_ioPortBusMaster =
        p_request->a_ioPortBusMaster;
    s_ataChannelContext[l_channelIndex].a_irq = p_request->a_irq;

    s_ataChannelCount++;

    // Create channel device
    dev_t l_deviceNumber = s_ataDeviceNumber;
    deviceSetMinor(&l_deviceNumber, l_channelIndex + 1);

    int l_returnValue =
        deviceAdd("ata", l_deviceNumber, &s_ataOperationsChannel, 1);

    if(l_returnValue != 0) {
        debug("ata: Failed to add channel device: %d.\n", l_returnValue);
        return l_returnValue;
    }

    l_returnValue = deviceCreateFile(l_deviceNumber);

    if(l_returnValue != 0) {
        debug(
            "ata: Failed to create channel device file: %d.\n",
            l_returnValue
        );

        return l_returnValue;
    }

    // Scan the channel for drives
    char l_driveFileName[NAME_MAX + 1];

    snprintf(l_driveFileName, NAME_MAX, "/dev/ata%d", l_channelIndex + 1);

    struct ts_vfsNode *l_drive;
    l_returnValue = vfsLookup(NULL, l_driveFileName, &l_drive);

    if(l_returnValue != 0) {
        debug(
            "ata: Failed to find %s: %d.\n",
            l_driveFileName,
            l_returnValue
        );

        return l_returnValue;
    }

    vfsOperationIoctl(l_drive, E_IOCTL_ATA_CHANNEL_SCAN, NULL);
    vfsOperationClose(l_drive);

    return 0;
}

static int ataIoctlChannel(
    struct ts_vfsNode *p_file,
    int p_request,
    void *p_arg
) {
    M_UNUSED_PARAMETER(p_arg);

    switch(p_request) {
        case E_IOCTL_ATA_CHANNEL_SCAN:
            ataIoctlChannelScan(p_file);
            return 0;

        default:
            return -EINVAL;
    }

    return -EINVAL;
}

static void ataIoctlChannelScan(struct ts_vfsNode *p_channelFile) {
    int l_channelNumber = deviceGetMinor(p_channelFile->a_deviceNumber) - 1;

    debug("ata: Scanning channel %d...\n", l_channelNumber);

    ataDriveScan(l_channelNumber, 0);
    ataDriveScan(l_channelNumber, 1);
}

static void ataDriveScan(int p_channelNumber, int p_driveNumber) {
    struct ts_ataChannel *l_channel =
        &s_ataChannelContext[p_channelNumber];
    struct ts_ataDrive *l_drive =
        &l_channel->a_drives[p_driveNumber];

    debug("ata: Scanning drive %d...\n", p_driveNumber);

    ataSelectDrive(p_channelNumber, p_driveNumber);

    outb(
        l_channel->a_ioPortBase + E_IOOFFSET_ATA_SECTOR_COUNT,
        0x00
    );
    outb(
        l_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_LOW,
        0x00
    );
    outb(
        l_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_MID,
        0x00
    );
    outb(
        l_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_HIGH,
        0x00
    );
    outb(
        l_channel->a_ioPortBase + E_IOOFFSET_ATA_COMMAND,
        E_ATA_COMMAND_IDENTIFY
    );

    ataWait(p_channelNumber);

    if(l_channel->a_registerCacheStatus == 0x00) {
        debug("ata: No drive detected.\n");
        return;
    }

    ataWaitUntilNotBusy(p_channelNumber);

    uint8_t l_lbaMid = inb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_MID);
    uint8_t l_lbaHigh = inb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_HIGH);
    uint16_t l_driveType = (l_lbaHigh << 8) | l_lbaMid;

    if(l_driveType == 0x0000) {
        debug("ata: Detected PATA drive.\n");

        ataWaitUntilDataRequestOrError(p_channelNumber);

        // Read identification data
        uint16_t l_identificationData[256];

        for(int l_index = 0; l_index < 256; l_index++) {
            l_identificationData[l_index] = inw(
                l_channel->a_ioPortBase + E_IOOFFSET_ATA_DATA
            );
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
            l_drive->a_usesLba = true;
            l_drive->a_usesLba48 = true;
            l_drive->a_sectorCount = l_totalSectorsLong;
        } else if(l_totalSectors != 0) {
            debug("ata: Drive supports LBA.\n");
            l_drive->a_usesLba = false;
            l_drive->a_sectorCount = l_totalSectors;

            if((l_identificationData[83] & (1 << 10)) != 0) {
                debug("ata: Drive supports LBA48.\n");
                l_drive->a_usesLba48 = true;
            } else {
                debug("ata: Drive supports LBA28.\n");
                l_drive->a_usesLba48 = false;
            }
        } else {
            debug("ata: Drive does not support LBA.\n");
            l_drive->a_usesLba = false;
            l_drive->a_usesLba48 = false;
            l_drive->a_cylinders = l_identificationData[1];
            l_drive->a_heads = l_identificationData[3];
            l_drive->a_sectors = l_identificationData[6];
            l_drive->a_sectorCount =
                l_drive->a_cylinders
                * l_drive->a_heads
                * l_drive->a_sectors;
        }

        // Create device
        dev_t l_deviceNumber = s_ataDeviceNumber;

        deviceSetMinor(
            &l_deviceNumber,
            3 + p_channelNumber * 2 + p_driveNumber
        );

        int l_returnValue = deviceAdd(
            "ata",
            l_deviceNumber,
            &s_ataOperationsDrivePata,
            1
        );

        if(l_returnValue != 0) {
            debug("ata: Failed to create drive device: %d.\n", l_returnValue);
            return;
        }

        l_returnValue = deviceCreateFile(l_deviceNumber);

        if(l_returnValue != 0) {
            debug(
                "ata: Failed to create drive device file: %d.\n",
                l_returnValue
            );

            return;
        }

        // Invalidate current drive
        l_channel->a_selectedDrive = -1;

        char l_deviceFileName[PATH_MAX];

        snprintf(
            l_deviceFileName,
            PATH_MAX - 1,
            "/dev/ata%d",
            deviceGetMinor(l_deviceNumber)
        );

        l_returnValue = vfsMount(l_deviceFileName, NULL, "mbrfs");

        if(l_returnValue != 0) {
            debug("ata: Failed to mount %s as mbrfs.\n", l_deviceFileName);
        }
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

static void ataSelectDrive(int p_channelNumber, int p_driveNumber) {
    struct ts_ataChannel *l_channel =
        &s_ataChannelContext[p_channelNumber];
    struct ts_ataDrive *l_drive =
        &l_channel->a_drives[p_driveNumber];

    if(l_channel->a_selectedDrive == p_driveNumber) {
        return;
    }

    if(p_driveNumber == 0) {
        l_channel->a_registerCacheDriveHead = 0xa0;
    } else {
        l_channel->a_registerCacheDriveHead = 0xb0;
    }

    if(l_drive->a_usesLba) {
        l_channel->a_registerCacheDriveHead |= 0x40;
    }

    outb(
        l_channel->a_ioPortBase + E_IOOFFSET_ATA_DRIVE_HEAD,
        l_channel->a_registerCacheDriveHead
    );

    ataWait(p_channelNumber);

    l_channel->a_selectedDrive = p_driveNumber;
}

static void ataWait(int p_channelNumber) {
    struct ts_ataChannel *l_channel =
        &s_ataChannelContext[p_channelNumber];

    for(int l_count = 0; l_count < 15; l_count++) {
        l_channel->a_registerCacheStatus =
            inb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_STATUS);
    }
}

static void ataWaitUntilNotBusy(int p_channelNumber) {
    struct ts_ataChannel *l_channel =
        &s_ataChannelContext[p_channelNumber];

    do {
        l_channel->a_registerCacheStatus =
            inb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_STATUS);
    } while((l_channel->a_registerCacheStatus & C_ATA_STATUS_MASK_BSY) != 0);
}

static void ataWaitUntilDataRequestOrError(int p_channelNumber) {
    struct ts_ataChannel *l_channel =
        &s_ataChannelContext[p_channelNumber];

    const uint8_t l_mask = C_ATA_STATUS_MASK_DRQ | C_ATA_STATUS_MASK_ERR;

    do {
        l_channel->a_registerCacheStatus =
            inb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_STATUS);
    } while((l_channel->a_registerCacheStatus & l_mask) == 0);
}

static int ataOpen(struct ts_vfsNode *p_file, int p_flags) {
    M_UNUSED_PARAMETER(p_file);
    M_UNUSED_PARAMETER(p_flags);

    return 0;
}

static ssize_t ataReadPata(
    struct ts_vfsNode *p_file,
    off_t p_offset,
    void *p_buffer,
    size_t p_size
) {
    M_UNUSED_PARAMETER(p_offset);

    int l_deviceMinor = deviceGetMinor(p_file->a_deviceNumber);

    int l_channelNumber = (l_deviceMinor - 3) / 2;
    int l_driveNumber = (l_deviceMinor - 3) % 2;

    // Compute offset
    size_t l_sector = p_offset >> 9;
    size_t l_sectorOffset = p_offset & 0x1ff;
    size_t l_bytesRead = 0;

    while(l_bytesRead < p_size) {
        uint8_t l_buffer[512];

        if(
            ataReadPataSector(
                l_channelNumber,
                l_driveNumber,
                l_sector,
                l_buffer
            ) != 512
        ) {
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
    struct ts_vfsNode *p_file,
    off_t p_offset,
    const void *p_buffer,
    size_t p_size
) {
    M_UNUSED_PARAMETER(p_file);
    M_UNUSED_PARAMETER(p_offset);
    M_UNUSED_PARAMETER(p_buffer);
    M_UNUSED_PARAMETER(p_size);

    return -EOPNOTSUPP;
}

static ssize_t ataReadPataSector(
    int p_channelNumber,
    int p_driveNumber,
    uint64_t p_lba,
    void *p_buffer
) {
    struct ts_ataChannel *l_channel =
        &s_ataChannelContext[p_channelNumber];
    struct ts_ataDrive *l_drive =
        &l_channel->a_drives[p_driveNumber];

    // Cannot read beyond drive capacity.
    if(p_lba >= l_drive->a_sectorCount) {
        return 0;
    }

    ataSelectDrive(p_channelNumber, p_driveNumber);
    ataWaitUntilNotBusy(p_channelNumber);

    // Set register values
    l_channel->a_registerCacheDriveHead &= 0xf0;

    if(l_drive->a_usesLba) {
        if(l_drive->a_usesLba48) {
            outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_DRIVE_HEAD, l_channel->a_registerCacheDriveHead);
            outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_SECTOR_COUNT, 0);
            outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_LOW, p_lba >> 24);
            outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_MID, p_lba >> 32);
            outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_HIGH, p_lba >> 40);
            outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_SECTOR_COUNT, 1);
            outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_LOW, p_lba);
            outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_MID, p_lba >> 8);
            outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_HIGH, p_lba >> 16);
            outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_COMMAND, E_ATA_COMMAND_READ_SECTORS_EXT);
        } else {
            l_channel->a_registerCacheDriveHead |= (p_lba >> 24) & 0x0f;
            outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_DRIVE_HEAD, l_channel->a_registerCacheDriveHead);
            ataWait(p_channelNumber);

            outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_SECTOR_COUNT, 1);
            outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_LOW, p_lba);
            outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_MID, p_lba >> 8);
            outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_LBA_HIGH, p_lba >> 16);
            outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_COMMAND, E_ATA_COMMAND_READ_SECTORS);
        }
    } else {
        uint32_t l_sectorsPerCylinder = l_drive->a_sectors * l_drive->a_heads;
        uint8_t l_head = (p_lba % l_sectorsPerCylinder) / l_drive->a_sectors;
        uint16_t l_cylinder = p_lba / l_sectorsPerCylinder;
        uint8_t l_sector = p_lba % l_drive->a_sectors + 1;

        l_channel->a_registerCacheDriveHead |= l_head & 0x0f;
        outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_DRIVE_HEAD, l_channel->a_registerCacheDriveHead);
        ataWait(p_channelNumber);

        outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_SECTOR_COUNT, 1);
        outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_SECTOR_NUMBER, l_sector);
        outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_CYLINDER_LOW, l_cylinder);
        outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_CYLINDER_HIGH, l_cylinder >> 8);
        outb(l_channel->a_ioPortBase + E_IOOFFSET_ATA_COMMAND, E_ATA_COMMAND_READ_SECTORS);
    }

    ataWaitUntilNotBusy(p_channelNumber);
    ataWaitUntilDataRequestOrError(p_channelNumber);

    if((l_channel->a_registerCacheDriveHead & C_ATA_STATUS_MASK_ERR) != 0) {
        return 0;
    }

    for(size_t l_bufferIndex = 0; l_bufferIndex < 256; l_bufferIndex++) {
        ((uint16_t *)p_buffer)[l_bufferIndex] = inw(l_channel->a_ioPortBase + E_IOOFFSET_ATA_DATA);
    }

    return 512;
}
