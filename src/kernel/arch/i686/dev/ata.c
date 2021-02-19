#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "time.h"
#include "arch/i686/io.h"
#include "arch/i686/dev/ata.h"
#include "dev/disk.h"
#include "libk/stdio.h"
#include "libk/stdlib.h"
#include "libk/string.h"

#define ATA_STATUS_BUSY 0x80
#define ATA_STATUS_DRIVE_READY 0x40
#define ATA_STATUS_DRIVE_WRITE_FAULT 0x20
#define ATA_STATUS_DRIVE_SEEK_COMPLETE 0x10
#define ATA_STATUS_DATA_REQUEST_READY 0x08
#define ATA_STATUS_DATA_CORRECTED 0x04
#define ATA_STATUS_INDEX 0x02
#define ATA_STATUS_ERROR 0x01

#define ATA_COMMAND_READ_PIO 0x20
#define ATA_COMMAND_READ_PIO_EXT 0x24
#define ATA_COMMAND_READ_DMA 0xc8
#define ATA_COMMAND_READ_DMA_EXT 0x25
#define ATA_COMMAND_WRITE_PIO 0x30
#define ATA_COMMAND_WRITE_PIO_EXT 0x34
#define ATA_COMMAND_WRITE_DMA 0xca
#define ATA_COMMAND_WRITE_DMA_EXT 0x35
#define ATA_COMMAND_CACHE_FLUSH 0xe7
#define ATA_COMMAND_CACHE_FLUSH_EXT 0xea
#define ATA_COMMAND_PACKET 0xa0
#define ATA_COMMAND_IDENTIFY_PACKET 0xa1
#define ATA_COMMAND_IDENTIFY 0xec

#define ATA_CTLREG_ALTERNATE_STATUS 0x0000
#define ATA_CTLREG_DEVICE_CONTROL 0x0000
#define ATA_CMDREG_DATA 0x0000
#define ATA_CMDREG_ERROR 0x0001
#define ATA_CMDREG_FEATURES 0x0001
#define ATA_CMDREG_SECTOR_COUNT 0x0002
#define ATA_CMDREG_SECTOR_NUMBER 0x0003
#define ATA_CMDREG_LBA_0_7 0x0003
#define ATA_CMDREG_CYLINDER_LOW 0x0004
#define ATA_CMDREG_LBA_8_15 0x0004
#define ATA_CMDREG_CYLINDER_HIGH 0x0005
#define ATA_CMDREG_LBA_16_23 0x0005
#define ATA_CMDREG_DRIVE_HEAD 0x0006
#define ATA_CMDREG_LBA_24_27 0x0006
#define ATA_CMDREG_STATUS 0x0007
#define ATA_CMDREG_COMMAND 0x0007

typedef struct {
    dev_disk_t disk;
    char deviceName[41];
    ata_channel_t *channel;
    int drive;
    bool lbaMode;
    uint16_t cylinders;
    uint8_t heads;
    uint8_t sectorsPerTrack;
} ata_disk_t;

typedef struct {
    uint16_t cylinder;
    uint8_t head;
    uint8_t sector;
} ata_chs_t;

void ata_init(ata_channel_t *channel, uint16_t io_ctl_base, uint16_t io_cmd_base);
static void ata_identify(ata_channel_t *channel, int drive);
static void ata_waitBsyEnd(ata_channel_t *channel);
static void ata_waitDrq(ata_channel_t *channel);
static void ata_receive(ata_channel_t *channel, void *buffer);
static void ata_selectDrive(ata_channel_t *channel, int drive);
static void ata_selectLba(ata_disk_t *disk, lba_t lba);
static void ata_lbaToChs(ata_chs_t *chs, ata_disk_t *disk, lba_t lba);
static int ata_api_read(ata_disk_t *disk, void *buffer, lba_t lba);
static int ata_api_write(ata_disk_t *disk, void *buffer, lba_t lba);
static void ata_api_getDeviceName(ata_disk_t *disk, char *buffer, size_t n);
static inline uint16_t read16(const void *buffer, size_t offset);
static inline uint32_t read32(const void *buffer, size_t offset);
static inline void readString(void *dst, const void *src, size_t n);

void ata_init(ata_channel_t *channel, uint16_t commandPort, uint16_t controlPort) {
    printf("ata: detected ATA channel (io_cmd=%#04x, io_ctl=%#04x)\n", commandPort, controlPort);

    channel->commandIoBase = commandPort;
    channel->controlIoBase = controlPort;
    channel->selectedDrive = -1;

    ata_identify(channel, 0);
    ata_identify(channel, 1);
}

static void ata_identify(ata_channel_t *channel, int drive) {
    uint8_t identify[512];

    ata_selectDrive(channel, drive);
    outb(channel->commandIoBase + ATA_CMDREG_SECTOR_COUNT, 0);
    outb(channel->commandIoBase + ATA_CMDREG_LBA_0_7, 0);
    outb(channel->commandIoBase + ATA_CMDREG_LBA_8_15, 0);
    outb(channel->commandIoBase + ATA_CMDREG_LBA_16_23, 0);
    outb(channel->commandIoBase + ATA_CMDREG_COMMAND, ATA_COMMAND_IDENTIFY);

    sleep(1);

    uint8_t status = inb(channel->commandIoBase + ATA_CMDREG_STATUS);

    if(!status) {
        printf("ata: no %s drive detected on this channel\n", drive ? "slave" : "master");
        return;
    }
    
    ata_waitBsyEnd(channel);

    if(inb(channel->commandIoBase + ATA_CMDREG_STATUS) & ATA_STATUS_ERROR) {
        uint8_t firstByte = inb(channel->commandIoBase + ATA_CMDREG_CYLINDER_LOW);
        uint8_t secondByte = inb(channel->commandIoBase + ATA_CMDREG_CYLINDER_HIGH);

        if(firstByte == 0x00 && secondByte == 0x00) {
            printf("ata: detected PATA device\n");
        } else if(firstByte == 0x14 && secondByte == 0xeb) {
            printf("ata: detected PATAPI device\n");
        } else if(firstByte == 0x3c && secondByte == 0xc3) {
            printf("ata: detected SATA device\n");
        } else if(firstByte == 0x69 && secondByte == 0x96) {
            printf("ata: detected SATAPI device\n");
        } else {
            printf("ata: unknown device identification: %#02x %#02x\n", firstByte, secondByte);
        }
        
        return;
    }

    ata_waitDrq(channel);

    ata_receive(channel, &identify);

    char buffer[41];

    readString(buffer, &identify[54], 40);

    printf("ata: registering device: \"%s\"\n", buffer);
    printf("ata: disk geometry: C=%d H=%d S=%d\n", read16(identify, 2), read16(identify, 6), read16(identify, 12));

    ata_disk_t *disk = malloc(sizeof(ata_disk_t));

    strncpy(disk->deviceName, (const char *)buffer, 40);
    disk->channel = channel;
    disk->drive = drive;
    disk->disk.hotplug = false;
    disk->disk.inserted = true;
    disk->disk.partition = false;
    disk->disk.writable = true;
    disk->disk.blockSize = 512;
    
    if(read16(identify, 98) & (1 << 9)) {
        disk->disk.blockCount = read32(identify, 120);
    } else {
        disk->disk.blockCount = read16(identify, 2) * read16(identify, 6) * read16(identify, 12);
    }
    
    disk->lbaMode = false;
    disk->cylinders = read16(identify, 2);
    disk->heads = read16(identify, 6);
    disk->sectorsPerTrack = read16(identify, 12);

    disk->disk.api.getDeviceName = ata_api_getDeviceName;
    disk->disk.api.read = ata_api_read;
    disk->disk.api.write = ata_api_write;

    disk_registerDevice((ata_disk_t *)disk);
}

static void ata_waitBsyEnd(ata_channel_t *channel) {
    while(inb(channel->commandIoBase + ATA_CMDREG_STATUS) & ATA_STATUS_BUSY);
}

static void ata_waitDrq(ata_channel_t *channel) {
    while(!(inb(channel->commandIoBase + ATA_CMDREG_STATUS) & ATA_STATUS_DATA_REQUEST_READY));
}

static void ata_receive(ata_channel_t *channel, void *buffer) {
    for(int i = 0; i < 256; i++) {
        ((uint16_t *)buffer)[i] = inw(channel->commandIoBase + ATA_CMDREG_DATA);
    }
}

static void ata_selectDrive(ata_channel_t *channel, int drive) {
    if(channel->selectedDrive != drive) {
        outb(channel->commandIoBase + ATA_CMDREG_DRIVE_HEAD, 0xa0 | ((drive & 1) << 4));
        sleep(1);
        channel->selectedDrive = drive;
    }
}

static void ata_lbaToChs(ata_chs_t *chs, ata_disk_t *disk, lba_t lba) {
    uint16_t sectorsPerCylinder = disk->sectorsPerTrack * disk->heads;

    chs->cylinder = lba / sectorsPerCylinder;
    chs->head = (lba % sectorsPerCylinder) / disk->sectorsPerTrack;
    chs->sector = (lba % disk->sectorsPerTrack) + 1;
}

static void ata_selectLba(ata_disk_t *disk, lba_t lba) {
    if(disk->lbaMode) {
        // TODO
    } else {
        ata_chs_t chs;

        ata_lbaToChs(&chs, disk, lba);

        outb(disk->channel->commandIoBase + ATA_CMDREG_SECTOR_NUMBER, chs.sector);
        outb(disk->channel->commandIoBase + ATA_CMDREG_CYLINDER_LOW, chs.cylinder);
        outb(disk->channel->commandIoBase + ATA_CMDREG_CYLINDER_HIGH, chs.cylinder >> 8);
        outb(disk->channel->commandIoBase + ATA_CMDREG_DRIVE_HEAD, (inb(disk->channel->commandIoBase + ATA_CMDREG_DRIVE_HEAD) & 0xf0) | (chs.head & 0x0f));
    }
}

static inline uint16_t read16(const void *buffer, size_t offset) {
    uint8_t *castedBuffer = (uint8_t *)buffer;

    return
        castedBuffer[offset]
        | (castedBuffer[offset + 1] << 8);
}

static inline uint32_t read32(const void *buffer, size_t offset) {
    uint8_t *castedBuffer = (uint8_t *)buffer;

    return 
        (castedBuffer[offset] << 16)
        | (castedBuffer[offset + 1] << 24)
        | castedBuffer[offset + 2]
        | (castedBuffer[offset + 3] << 8);
}

static inline void readString(void *dst, const void *src, size_t n) {
    uint16_t *castedSrc = (uint16_t *)src;
    uint16_t *castedDst = (uint16_t *)dst;
    
    n >>= 1;

    for(size_t i = 0; i < n; i++) {
        castedDst[i] = (castedSrc[i] >> 8) | (castedSrc[i] << 8);
    }

    ((uint8_t *)dst)[n << 1] = '\0';
}

static int ata_api_read(ata_disk_t *disk, void *buffer, lba_t lba) {
    ata_selectDrive(disk->channel, disk->drive);
    outb(disk->channel->commandIoBase + ATA_CMDREG_SECTOR_COUNT, 1);
    ata_selectLba(disk, lba);

    outb(disk->channel->commandIoBase + ATA_CMDREG_COMMAND, ATA_COMMAND_READ_PIO);

    ata_waitBsyEnd(disk->channel);

    uint8_t status = inb(disk->channel->commandIoBase + ATA_CMDREG_STATUS);

    ata_waitDrq(disk->channel);

    ata_receive(disk->channel, buffer);

    return 0;
}

static int ata_api_write(ata_disk_t *disk, void *buffer, lba_t lba) {
    ata_selectDrive(disk->channel, disk->drive);

    return 1;
}

static void ata_api_getDeviceName(ata_disk_t *disk, char *buffer, size_t n) {
    strncpy(buffer, disk->deviceName, n);
}
