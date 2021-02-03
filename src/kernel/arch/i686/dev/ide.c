#include <stdbool.h>
#include <stdint.h>

#include "arch/i686/dev/ide.h"
#include "arch/i686/io.h"
#include "libk/stdio.h"
#include "libk/stdlib.h"

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

ide_controller_t *ide_constructor(uint16_t primaryControl, uint16_t primaryData, uint16_t secondaryControl, uint16_t secondaryData, uint16_t busMaster);
void ide_init(ide_controller_t *controller);

ide_controller_t *ide_constructor(uint16_t primaryControl, uint16_t primaryData, uint16_t secondaryControl, uint16_t secondaryData, uint16_t busMaster) {
    ide_controller_t *controller = malloc(sizeof(ide_controller_t));
    controller->primaryChannel.controlPort = primaryControl;
    controller->primaryChannel.dataPort = primaryData;
    controller->secondaryChannel.controlPort = secondaryControl;
    controller->secondaryChannel.dataPort = secondaryData;
    controller->busMasterPort = busMaster;

    return controller;
}

void ide_init(ide_controller_t *controller) {
    printf("ide: new controller: 1c=%#04x, 1d=%#04x, 2c=%#04x, 2d=%#04x, bm=%#04x\n", controller->primaryChannel.controlPort, controller->primaryChannel.dataPort, controller->secondaryChannel.controlPort, controller->secondaryChannel.dataPort, controller->busMasterPort);
}
