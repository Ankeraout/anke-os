#ifndef __KERNEL_ARCH_I686_DEV_IDE_H__
#define __KERNEL_ARCH_I686_DEV_IDE_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint16_t controlPort;
    uint16_t dataPort;
} ide_controller_channel_t;

typedef struct {
    ide_controller_channel_t primaryChannel;
    ide_controller_channel_t secondaryChannel;
    uint16_t busMasterPort;
} ide_controller_t;

typedef struct {
    ide_controller_t *controller;
    bool secondaryChannel;
    bool slave;
} ide_drive_t;

ide_controller_t *ide_constructor(uint16_t primaryControl, uint16_t primaryData, uint16_t secondaryControl, uint16_t secondaryData, uint16_t busMaster);
void ide_init(ide_controller_t *controller);

#endif
