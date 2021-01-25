#ifndef __KERNEL_ACPI_RSDP_H__
#define __KERNEL_ACPI_RSDP_H__

#include <stdint.h>

typedef struct {
    uint8_t signature[8];
    uint8_t checksum;
    uint8_t oemid[6];
    uint8_t revision;
    uint32_t rsdtPtr;
    uint32_t length;
    uint64_t xsdtPtr;
    uint8_t extendedChecksum;
    uint8_t reserved[3];
} acpi_rsdp_t;

bool acpi_rsdp_isChecksumValid(acpi_rsdp_t *rsdp);
const acpi_rsdp_t *acpi_rsdp_locate();
void acpi_rsdp_unmap();

#endif
