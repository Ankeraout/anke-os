#ifndef __KERNEL_ARCH_I686_DEV_ATA_H__
#define __KERNEL_ARCH_I686_DEV_ATA_H__

#include <stdint.h>

typedef enum {
    ATA_DEVICETYPE_NONE,
    ATA_DEVICETYPE_PATA,
    ATA_DEVICETYPE_PATAPI,
    ATA_DEVICETYPE_SATA,
    ATA_DEVICETYPE_SATAPI
} ata_deviceType_t;

typedef struct {
    ata_deviceType_t deviceType;
    
} ata_device_t;

typedef struct {
    uint16_t controlIoBase;
    uint16_t commandIoBase;
    int selectedDrive;
    ata_device_t devices[2];
} ata_channel_t;

void ata_init(ata_channel_t *bus, uint16_t commandPort, uint16_t controlPort);

#endif
