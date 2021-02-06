#include <stdint.h>

#include "arch/i686/io.h"
#include "arch/i686/dev/ata.h"
#include "libk/stdio.h"

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

void ata_init(ata_channel_t *channel, uint16_t io_ctl_base, uint16_t io_cmd_base);
static void ata_identify(ata_channel_t *channel, int drive);
static void ata_waitBsyEnd(ata_channel_t *channel);
static void ata_receive(ata_channel_t *channel, void *buffer);
static void ata_selectDrive(ata_channel_t *channel, int drive);

void ata_init(ata_channel_t *channel, uint16_t commandPort, uint16_t controlPort) {
    printf("ata: Detected ATA channel (io_cmd=%#04x, io_ctl=%#04x)\n", commandPort, controlPort);

    /*
    ata_identify(channel, 0);
    ata_identify(channel, 1);
    */
}

static void ata_identify(ata_channel_t *channel, int drive) {
    ata_identify_t identify;

    uint8_t tmp = inb(channel->commandIoBase + ATA_CMDREG_DRIVE_HEAD);
    tmp &= ~(1 << 4);
    tmp |= (drive & 1) << 4;
    outb(channel->commandIoBase + ATA_CMDREG_DRIVE_HEAD, tmp);

    ata_waitBsyEnd(channel);

    printf("ata: sending IDENTIFY command...\n");
    outb(channel->commandIoBase + ATA_CMDREG_COMMAND, ATA_COMMAND_IDENTIFY);

    printf("ata: waiting for drive to be ready...\n");
    while(!(inb(channel->commandIoBase + ATA_CMDREG_STATUS) & ATA_STATUS_DATA_REQUEST_READY));

    printf("ata: receiving IDENTIFY data...\n");
    ata_receive(channel, &identify);

    printf("ata: found device: %.40s\n", identify.modelNumber);
}

static void ata_waitBsyEnd(ata_channel_t *channel) {
    printf("ata: waiting for BSY bit to clear...\n");
    while(inb(channel->commandIoBase + ATA_CMDREG_STATUS) & ATA_STATUS_BUSY);
    printf("ata: BSY bit cleared\n");
}

static void ata_receive(ata_channel_t *channel, void *buffer) {
    for(int i = 0; i < 256; i++) {
        uint16_t tmp = inw(channel->commandIoBase + ATA_CMDREG_DATA);
        ((uint16_t *)buffer)[i] = (tmp >> 8) | (tmp << 8);
    }
}
