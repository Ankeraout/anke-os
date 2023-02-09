#include <stdbool.h>
#include <stdint.h>

#include "arch/x86/inline.h"
#include "dev/ide.h"
#include "debug.h"

#define C_IDE_STATUS_MASK_ERR 0x01
#define C_IDE_STATUS_MASK_IDX 0x02
#define C_IDE_STATUS_MASK_CORR 0x04
#define C_IDE_STATUS_MASK_DRQ 0x08
#define C_IDE_STATUS_MASK_SRV 0x10
#define C_IDE_STATUS_MASK_DF 0x20
#define C_IDE_STATUS_MASK_RDY 0x40
#define C_IDE_STATUS_MASK_BSY 0x80

enum te_devIdeIoPortOffset {
    E_IDE_IOOFFSET_DATA = 0,
    E_IDE_IOOFFSET_ERROR = 1,
    E_IDE_IOOFFSET_FEATURES = 1,
    E_IDE_IOOFFSET_SECTOR_COUNT = 2,
    E_IDE_IOOFFSET_LBA_LOW = 3,
    E_IDE_IOOFFSET_SECTOR_NUMBER = 3,
    E_IDE_IOOFFSET_LBA_MID = 4,
    E_IDE_IOOFFSET_CYLINDER_LOW = 4,
    E_IDE_IOOFFSET_LBA_HIGH = 5,
    E_IDE_IOOFFSET_CYLINDER_HIGH = 5,
    E_IDE_IOOFFSET_DRIVE_HEAD = 6,
    E_IDE_IOOFFSET_STATUS = 7,
    E_IDE_IOOFFSET_COMMAND = 7
};

enum te_devIdeCommand {
    E_IDE_COMMAND_READ_SECTORS = 0x20,
    E_IDE_COMMAND_READ_SECTORS_EXT = 0x24,
    E_IDE_COMMAND_IDENTIFY = 0xec
};

static void ideWait(struct ts_devIde *p_dev);
static void ideDetectDrive(struct ts_devIde *p_dev, enum te_devIdeDrive p_drive);
static void ideSelectDrive(struct ts_devIde *p_dev, enum te_devIdeDrive p_drive);
static void ideWaitBusy(struct ts_devIde *p_dev);
static void ideWaitDataRequestOrError(struct ts_devIde *p_dev);
static bool ideIsDriveNumberValid(enum te_devIdeDrive p_drive);
static int idePataRead(
    void *p_buffer,
    struct ts_devIde *p_dev,
    int p_drive,
    t_devIdeLba p_lba,
    size_t p_sectorCount
);

int ideInit(struct ts_devIde *p_dev) {
    debugPrint("ide: Initializing channel (I/O bases: 0x");
    debugPrintHex16(p_dev->a_ioPortBase);
    debugPrint(", 0x");
    debugPrintHex16(p_dev->a_ioPortControl);
    debugPrint(").\n");

    // Initialize structure
    p_dev->a_registerCacheDriveHead = 0xa0;
    p_dev->a_selectedDrive = 0;

    // Detect drives
    ideDetectDrive(p_dev, E_DEV_IDE_DRIVE_MASTER);
    ideDetectDrive(p_dev, E_DEV_IDE_DRIVE_SLAVE);

    return 0;
}

int ideRead(
    void *p_buffer,
    struct ts_devIde *p_dev,
    int p_drive,
    t_devIdeLba p_lba,
    size_t p_sectorCount
) {
    // We only support reading from PATA devices.
    if(p_dev->a_drives[p_drive].a_type != E_DEV_IDE_DRIVETYPE_PATA) {
        return 1;
    }

    return idePataRead(p_buffer, p_dev, p_drive, p_lba, p_sectorCount);
}

static int idePataRead(
    void *p_buffer,
    struct ts_devIde *p_dev,
    int p_drive,
    t_devIdeLba p_lba,
    size_t p_sectorCount
) {
    // Cannot read if there is no PATA drive
    if(p_dev->a_drives[p_drive].a_type != E_DEV_IDE_DRIVETYPE_PATA) {
        debugPrint("ide: Cannot read from non-PATA or absent drive.\n");
        return 1;
    }

    // We cannot read beyond the drive capacity
    if(p_lba >= p_dev->a_drives[p_drive].a_sectorCount) {
        debugPrint("ide: Cannot read beyond disk capacity.\n");
        return 1;
    }

    // If the user wants to read 0 sectors, then there is nothing to do.
    if(p_sectorCount == 0) {
        return 0;
    }

    // We cannot read more than 65535 sectors at a time.
    if(p_sectorCount > 65535) {
        debugPrint("ide: Tried to read too many sectors at a time.\n");
        return 1;
    }

    // If we are not using LBA48, we can only read 255 sectors at a time.
    if((!p_dev->a_drives[p_drive].a_usesLba48) && (p_sectorCount > 255)) {
        debugPrint("ide: Tried to read too many sectors at a time.\n");
        return 1;
    }

    // Select drive
    ideSelectDrive(p_dev, p_drive);

    // Wait for the drive to be ready
    ideWaitBusy(p_dev);

    // Set the registers
    p_dev->a_registerCacheDriveHead &= 0xf0;

    if(p_dev->a_drives[p_drive].a_usesLba) {
        if(p_dev->a_drives[p_drive].a_usesLba48) {
            outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_DRIVE_HEAD, p_dev->a_registerCacheDriveHead);
            ideWait(p_dev);

            outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_SECTOR_COUNT, p_sectorCount >> 8);
            outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_LBA_LOW, p_lba >> 24);
            outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_LBA_MID, p_lba >> 32);
            outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_LBA_HIGH, p_lba >> 40);
            outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_SECTOR_COUNT, p_sectorCount);
            outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_LBA_LOW, p_lba);
            outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_LBA_MID, p_lba >> 8);
            outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_LBA_HIGH, p_lba >> 16);
            outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_COMMAND, E_IDE_COMMAND_READ_SECTORS_EXT);
        } else {
            p_dev->a_registerCacheDriveHead |= (p_lba >> 24) & 0x0f;
            outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_DRIVE_HEAD, p_dev->a_registerCacheDriveHead);
            ideWait(p_dev);

            outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_SECTOR_COUNT, p_sectorCount);
            outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_LBA_LOW, p_lba & 0xff);
            outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_LBA_MID, (p_lba >> 8) & 0xff);
            outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_LBA_HIGH, (p_lba >> 16) & 0xff);
            outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_COMMAND, E_IDE_COMMAND_READ_SECTORS);
        }
    } else {
        // TODO: compute CHS
        uint32_t l_sectorsPerCylinder = p_dev->a_drives[p_drive].a_sectors * p_dev->a_drives[p_drive].a_heads;
        uint8_t l_head = (p_lba % l_sectorsPerCylinder) / p_dev->a_drives[p_drive].a_sectors;
        uint16_t l_cylinder = p_lba / l_sectorsPerCylinder;
        uint8_t l_sector = p_lba % p_dev->a_drives[p_drive].a_sectors + 1;

        p_dev->a_registerCacheDriveHead |= l_head & 0x0f;
        outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_DRIVE_HEAD, p_dev->a_registerCacheDriveHead);
        ideWait(p_dev);

        outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_SECTOR_COUNT, p_sectorCount);
        outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_SECTOR_NUMBER, l_sector);
        outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_CYLINDER_LOW, l_cylinder);
        outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_CYLINDER_HIGH, l_cylinder >> 8);
        outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_COMMAND, E_IDE_COMMAND_READ_SECTORS);
    }

    size_t l_bufferIndex = 0;
    uint16_t *l_buffer = (uint16_t *)p_buffer;

    for(size_t l_sector = 0; l_sector < p_sectorCount; l_sector++) {
        ideWaitBusy(p_dev);
        ideWaitDataRequestOrError(p_dev);

        // If a read error occurred, return an error code.
        if((p_dev->a_registerCacheStatus & C_IDE_STATUS_MASK_ERR) != 0) {
            debugPrint("ide: Error while reading from disk.\n");
            return 1;
        }

        // Read the sector
        for(size_t l_word = 0; l_word < 256; l_word++) {
            l_buffer[l_bufferIndex++] = inw(p_dev->a_ioPortBase + E_IDE_IOOFFSET_DATA);
        }
    }

    return 0;
}

static void ideWait(struct ts_devIde *p_dev) {
    for(int l_count = 0; l_count < 15; l_count++) {
        inb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_STATUS);
    }
}

static void ideDetectDrive(struct ts_devIde *p_dev, enum te_devIdeDrive p_drive) {
    ideSelectDrive(p_dev, p_drive);

    debugPrint("ide: Checking for ");
    debugPrint(p_drive == 0 ? "master" : "slave");
    debugPrint(" drive...\n");

    outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_SECTOR_COUNT, 0x00);
    outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_LBA_LOW, 0x00);
    outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_LBA_MID, 0x00);
    outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_LBA_HIGH, 0x00);
    outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_COMMAND, E_IDE_COMMAND_IDENTIFY);

    ideWait(p_dev);

    if(inb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_STATUS) == 0x00) {
        p_dev->a_drives[p_drive].a_type = E_DEV_IDE_DRIVETYPE_NONE;
        debugPrint("ide: No drive detected.\n");
        return;
    }

    ideWaitBusy(p_dev);

    // If LBA_MID or LBA_HIGH ports are non-zero, the drive is not an ATA drive.
    uint8_t l_lbaMid = inb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_LBA_MID);
    uint8_t l_lbaHigh = inb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_LBA_HIGH);
    uint16_t l_driveType = (l_lbaHigh << 8) | l_lbaMid;

    if(l_driveType != 0) {
        debugPrint("ide: Detected ");

        if(l_driveType == 0xeb14) {
            p_dev->a_drives[p_drive].a_type = E_DEV_IDE_DRIVETYPE_PATAPI;
            debugPrint("PATAPI");
        } else if(l_driveType == 0x9669)  {
            p_dev->a_drives[p_drive].a_type = E_DEV_IDE_DRIVETYPE_SATAPI;
            debugPrint("SATAPI");
        } else if(l_driveType == 0xc33c) {
            p_dev->a_drives[p_drive].a_type = E_DEV_IDE_DRIVETYPE_SATA;
            debugPrint("SATA");
        }

        debugPrint(" drive.\n");

        return;
    }

    p_dev->a_drives[p_drive].a_type = E_DEV_IDE_DRIVETYPE_PATA;
    debugPrint("ide: Detected PATA drive.\n");

    ideWaitDataRequestOrError(p_dev);

    // Check for errors
    if((p_dev->a_registerCacheStatus & C_IDE_STATUS_MASK_ERR) != 0) {
        debugPrint("ide: Error while identifying the drive.\n");
        return;
    }

    // Read identification data
    uint16_t l_identificationData[256];

    for(int l_index = 0; l_index < 256; l_index++) {
        l_identificationData[l_index] = inw(p_dev->a_ioPortBase + E_IDE_IOOFFSET_DATA);
    }

    debugPrint("ide: Drive name: ");

    for(int l_index = 0; l_index < 20; l_index++) {
        uint16_t l_word = l_identificationData[27 + l_index];

        char l_buffer[] = {l_word >> 8, l_word, 0};

        debugPrint(l_buffer);
    }

    debugPrint("\n");

    const uint64_t l_totalSectorsLong = *(const uint64_t *)&l_identificationData[100];
    const uint32_t l_totalSectors = *(const uint32_t *)&l_identificationData[60];

    if(l_totalSectorsLong != 0) {
        debugPrint("ide: Drive supports LBA48.\n");
        p_dev->a_drives[p_drive].a_usesLba = true;
        p_dev->a_drives[p_drive].a_usesLba48 = true;
        p_dev->a_drives[p_drive].a_sectorCount = l_totalSectorsLong;
    } else if(l_totalSectors != 0) {
        debugPrint("ide: Drive supports LBA.\n");
        p_dev->a_drives[p_drive].a_usesLba = true;

        if((l_identificationData[83] & (1 << 10)) != 0) {
            debugPrint("ide: Drive supports LBA48.\n");
            p_dev->a_drives[p_drive].a_usesLba48 = true;
        } else {
            debugPrint("ide: Drive does not support LBA48.\n");
            p_dev->a_drives[p_drive].a_usesLba48 = false;
        }

        p_dev->a_drives[p_drive].a_sectorCount = l_totalSectors;
    } else {
        debugPrint("ide: Drive does not support LBA.\n");
        p_dev->a_drives[p_drive].a_usesLba = false;
        p_dev->a_drives[p_drive].a_sectorCount =
            p_dev->a_drives[p_drive].a_cylinders
            * p_dev->a_drives[p_drive].a_heads
            * p_dev->a_drives[p_drive].a_sectors;
    }

    p_dev->a_drives[p_drive].a_cylinders = l_identificationData[1];
    p_dev->a_drives[p_drive].a_heads = l_identificationData[3];
    p_dev->a_drives[p_drive].a_sectors = l_identificationData[6];

    debugPrint("ide: Cylinders: 0x");
    debugPrintHex32(p_dev->a_drives[p_drive].a_cylinders);
    debugPrint(", heads: 0x");
    debugPrintHex8(p_dev->a_drives[p_drive].a_heads);
    debugPrint(", sectors: 0x");
    debugPrintHex8(p_dev->a_drives[p_drive].a_sectors);
    debugPrint("\n");
    debugPrint("ide: Total sector count: 0x");
    debugPrintHex64(p_dev->a_drives[p_drive].a_sectorCount);
    debugPrint("\n");
}

static void ideSelectDrive(struct ts_devIde *p_dev, enum te_devIdeDrive p_drive) {
    if(p_dev->a_selectedDrive == p_drive) {
        return;
    }

    p_dev->a_registerCacheDriveHead = 0xa0;

    if(p_dev->a_drives[p_drive].a_usesLba) {
        p_dev->a_registerCacheDriveHead |= 0x40;
    }

    if(p_drive == E_DEV_IDE_DRIVE_SLAVE) {
        p_dev->a_registerCacheDriveHead |= 0x10;
    }

    outb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_DRIVE_HEAD, p_dev->a_registerCacheDriveHead);

    ideWait(p_dev);

    p_dev->a_selectedDrive = p_drive;
}

static void ideWaitBusy(struct ts_devIde *p_dev) {
    do {
        p_dev->a_registerCacheStatus = inb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_STATUS);
    } while((p_dev->a_registerCacheStatus & C_IDE_STATUS_MASK_BSY) != 0);
}

static void ideWaitDataRequestOrError(struct ts_devIde *p_dev) {
    const uint8_t l_mask = C_IDE_STATUS_MASK_DRQ | C_IDE_STATUS_MASK_ERR;

    do {
        p_dev->a_registerCacheStatus = inb(p_dev->a_ioPortBase + E_IDE_IOOFFSET_STATUS);
    } while((p_dev->a_registerCacheStatus & l_mask) == 0);
}

static bool ideIsDriveNumberValid(enum te_devIdeDrive p_drive) {
    return (
        (p_drive == E_DEV_IDE_DRIVE_MASTER)
        || (p_drive == E_DEV_IDE_DRIVE_SLAVE)
    );
}
