#ifndef __KERNEL_ARCH_I686_DEV_ATA_H__
#define __KERNEL_ARCH_I686_DEV_ATA_H__

#include <stdint.h>

typedef struct {
    union {
        struct {
            uint16_t reserved : 1;
            uint16_t vendorSpecific0 : 5;
            uint16_t notRemovableController : 1;
            uint16_t notRemovableDevice : 1;
            uint16_t vendorSpecific1 : 7;
            uint16_t zero : 1;
        } __attribute__((packed)) flags;

        uint16_t value;
    } generalConfiguration;

    uint16_t logicalCylinderCount;
    uint16_t reserved0;
    uint16_t logicalHeadCount;
    uint16_t vendorSpecific0[2];
    uint16_t sectorsPerTrack;
    uint16_t vendorSpecific1[3];
    uint16_t serialNumber[10];
    uint16_t vendorSpecific2[2];
    uint16_t vendorSpecificByteCount;
    uint8_t firmwareVersion[8];
    uint8_t modelNumber[40];
    
    union {
        struct {
            uint16_t maximumSectorCount : 8;
            uint16_t vendorSpecific : 8;
        } __attribute__((packed)) flags;

        uint16_t value;
    } rwMultipleCapabilitiesPerInterrupt0;

    uint16_t reserved1;

    union {
        struct {
            uint16_t vendorSpecific : 8;
            uint16_t dmaSupported : 1;
            uint16_t lbaSupported : 1;
            uint16_t iordyDisablable : 1;
            uint16_t iordySupported : 1;
            uint16_t reserved0 : 1;
            uint16_t standbyTimerValuesStandard : 1;
            uint16_t reserved1 : 2;
        } __attribute__((packed)) flags;

        uint16_t value;
    } capabilities;

    uint16_t reserved2;
    
    union {
        struct {
            uint16_t vendorSpecific : 8;
            uint16_t timingMode : 8;
        } __attribute__((packed)) flags;

        uint16_t value;
    } pioTimingMode;
    
    union {
        struct {
            uint16_t vendorSpecific : 8;
            uint16_t timingMode : 8;
        } __attribute__((packed)) flags;

        uint16_t value;
    } dmaTimingMode;

    union {
        struct {
            uint16_t chsInformationValid : 1;
            uint16_t cycleTimeInformationValid : 1;
            uint16_t reserved : 14;
        } __attribute__((packed)) flags;

        uint16_t value;
    } fieldValidity;

    uint16_t currentLogicalCylinderCount;
    uint16_t currentLogicalHeadCount;
    uint16_t currentLogicalSectorsPerTrack;
    uint16_t currentCapacityInSectors0;
    uint16_t currentCapacityInSectors1;
    
    union {
        struct {
            uint16_t maximumSectorCount : 8;
            uint16_t valid : 1;
            uint16_t vendorSpecific : 7;
        } __attribute__((packed)) flags;

        uint16_t value;
    } rwMultipleCapabilitiesPerInterrupt1;

    uint16_t userAddressableSectors0;
    uint16_t userAddressableSectors1;

    union {
        struct {
            uint16_t supported : 8;
            uint16_t active : 8;
        } __attribute__((packed)) flags;

        uint16_t value;
    } singleWordDmaTransferMode;

    union {
        struct {
            uint16_t supported : 8;
            uint16_t active : 8;
        } __attribute__((packed)) flags;

        uint16_t value;
    } multiWordDmaTransferMode;

    union {
        struct {
            uint16_t supported : 8;
            uint16_t reserved : 8;
        } __attribute__((packed)) flags;

        uint16_t value;
    } advancedPioTransferMode;

    uint16_t minimumMultiwordDmaTransferCycleTimePerWord;
    uint16_t recommendedMultiwordDmaTransferCycleTime;
    uint16_t minimumPioTransferCycleTimeWithoutFlowControl;
    uint16_t minimumPioTransferCycleTimeWithIordyFlowControl;
    uint16_t reserved3[2];
    uint16_t reserved4[57];
    uint16_t vendorSpecific[32];
    uint16_t reserved5[96];
} __attribute__((packed)) ata_identify_t;

typedef enum {
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
