#include <stdint.h>

#include "time.h"
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
static void ata_waitDrq(ata_channel_t *channel);
static void ata_receive(ata_channel_t *channel, void *buffer);
static void ata_wait(ata_channel_t *channel);

void ata_init(ata_channel_t *channel, uint16_t commandPort, uint16_t controlPort) {
    printf("ata: detected ATA channel (io_cmd=%#04x, io_ctl=%#04x)\n", commandPort, controlPort);

    channel->commandIoBase = commandPort;
    channel->controlIoBase = controlPort;
    channel->selectedDrive = -1;

    ata_identify(channel, 0);
    ata_identify(channel, 1);
}

static void ata_identify(ata_channel_t *channel, int drive) {
    ata_identify_t identify;

    outb(channel->commandIoBase + ATA_CMDREG_DRIVE_HEAD, 0xa0 | ((drive & 1) << 4));

    sleep(1);

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

    printf("ata: device identification: %.40s\n", identify.modelNumber);
}

static void ata_waitBsyEnd(ata_channel_t *channel) {
    while(inb(channel->commandIoBase + ATA_CMDREG_STATUS) & ATA_STATUS_BUSY);
}

static void ata_waitDrq(ata_channel_t *channel) {
    while(!(inb(channel->commandIoBase + ATA_CMDREG_STATUS) & ATA_STATUS_DATA_REQUEST_READY));
}

static void ata_receive(ata_channel_t *channel, void *buffer) {
    for(int i = 0; i < 256; i++) {
        uint16_t tmp = inw(channel->commandIoBase + ATA_CMDREG_DATA);
        ((uint16_t *)buffer)[i] = (tmp >> 8) | (tmp << 8);
    }
}
