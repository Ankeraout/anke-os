#ifndef __KERNEL_ACPI_ACPI_H__
#define __KERNEL_ACPI_ACPI_H__

#include <stdint.h>

typedef struct {
    uint8_t addressSpace;
    uint8_t bitWidth;
    uint8_t bitOffset;
    uint8_t accessSize;
    uint64_t address;
} __attribute__((packed)) acpi_genericAddressStructure_t;

#endif
