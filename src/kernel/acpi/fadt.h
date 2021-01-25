#ifndef __KERNEL_ACPI_FADT_H__
#define __KERNEL_ACPI_FADT_H__

#include <stdint.h>

#include "acpi/acpi.h"
#include "acpi/sdt.h"

enum {
    ACPI_FADT_GAS_ADDRESSSPACE_SYSTEM_MEMORY,
    ACPI_FADT_GAS_ADDRESSSPACE_SYSTEM_IO,
    ACPI_FADT_GAS_ADDRESSSPACE_PCI_CS,
    ACPI_FADT_GAS_ADDRESSSPACE_EMBEDDED_CONTROLLER,
    ACPI_FADT_GAS_ADDRESSSPACE_SMBUS,
    ACPI_FADT_GAS_ADDRESSSPACE_SYSTEM_CMOS,
    ACPI_FADT_GAS_ADDRESSSPACE_PCI_DEVICE_BAR,
    ACPI_FADT_GAS_ADDRESSSPACE_IPMI,
    ACPI_FADT_GAS_ADDRESSSPACE_GPIO,
    ACPI_FADT_GAS_ADDRESSSPACE_GENERIC_SERIAL_BUS,
    ACPI_FADT_GAS_ADDRESSSPACE_PLATFORM_CONFIGURATION_CHANNEL
};

enum {
    ACPI_FADT_GAS_ACCESSSIZE_UNDEFINED,
    ACPI_FADT_GAS_ACCESSSIZE_8,
    ACPI_FADT_GAS_ACCESSSIZE_16,
    ACPI_FADT_GAS_ACCESSSIZE_32,
    ACPI_FADT_GAS_ACCESSSIZE_64
};

enum {
    ACPI_FADT_PMP_UNSPECIFIED,
    ACPI_FADT_PMP_DESKTOP,
    ACPI_FADT_PMP_MOBILE,
    ACPI_FADT_PMP_WORKSTATION,
    ACPI_FADT_PMP_ENTREPRISE_SERVER,
    ACPI_FADT_PMP_SOHO_SERVER,
    ACPI_FADT_PMP_APPLIANCE_PC,
    ACPI_FADT_PMP_PERFORMANCE_SERVER
};

typedef struct {
    acpi_sdt_header_t header;
    uint32_t firmwareCtrl;
    uint32_t dsdtPtr;
    uint8_t reserved;
    uint8_t preferredPowerManagementProfile;
    uint16_t sciInterrupt;
    uint32_t smiCommandPort;
    uint8_t acpiEnable;
    uint8_t acpiDisable;
    uint8_t s4biosReq;
    uint8_t pStateControl;
    uint32_t pm1aEventBlock;
    uint32_t pm1bEventBlock;
    uint32_t pm1aControlBlock;
    uint32_t pm1bControlBlock;
    uint32_t pm2ControlBlock;
    uint32_t pmTimerBlock;
    uint32_t gpe0block;
    uint32_t gpe1block;
    uint8_t pm1EventLength;
    uint8_t pm1ControlLength;
    uint8_t pm2ControlLength;
    uint8_t pmTimerLength;
    uint8_t gpe0Length;
    uint8_t gpe1Length;
    uint8_t gpe1Base;
    uint8_t cStateControl;
    uint16_t worstC2Latency;
    uint16_t worstC3Latency;
    uint16_t flushSize;
    uint16_t flushStride;
    uint8_t dutyOffset;
    uint8_t dutyWidth;
    uint8_t dayAlarm;
    uint8_t monthAlarm;
    uint8_t century;
    uint16_t bootArchitectureFlags;
    uint8_t reserved2;
    uint32_t flags;
    acpi_genericAddressStructure_t resetReg;
    uint8_t resetValue;
    uint8_t reserved3[3];
    uint64_t xFirmwareControl;
    uint64_t xDsdt;
    acpi_genericAddressStructure_t xPm1aEventBlock;
    acpi_genericAddressStructure_t xPm1bEventBlock;
    acpi_genericAddressStructure_t xPm1aControlBlock;
    acpi_genericAddressStructure_t xPm1bControlBlock;
    acpi_genericAddressStructure_t xPm2ControlBlock;
    acpi_genericAddressStructure_t xPmTimerBlock;
    acpi_genericAddressStructure_t xGpe0Block;
    acpi_genericAddressStructure_t xGpe1Block;
} __attribute__((packed)) acpi_fadt_t;

#endif
