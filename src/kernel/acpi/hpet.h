#ifndef __KERNEL_ACPI_HPET_H__
#define __KERNEL_ACPI_HPET_H__

#include <stdint.h>

#include "acpi/acpi.h"
#include "acpi/sdt.h"

typedef struct {
    acpi_sdt_header_t header;
    uint8_t hardwareRevId;
    uint8_t comparatorCount : 5;
    uint8_t counterSize : 1;
    uint8_t reserved : 1;
    uint8_t legacyReplacement : 1;
    uint16_t pciVendorId;
    acpi_genericAddressStructure_t address;
    uint8_t hpetNumber;
    uint16_t minimumTick;
    uint8_t pageProtection;
} __attribute__((packed)) acpi_hpet_t;

#endif
