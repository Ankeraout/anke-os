#ifndef __KERNEL_ARCH_I686_ACPI_SDT_H__
#define __KERNEL_ARCH_I686_ACPI_SDT_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oemId[6];
    uint8_t oemTableId[8];
    uint32_t oemRevision;
    uint32_t creatorId;
    uint32_t creatorRevision;
} __attribute__((packed)) acpi_sdt_header_t;

typedef struct {
    acpi_sdt_header_t header;
    uint8_t reserved[];
} __attribute__((packed)) acpi_sdt_t;

bool acpi_sdt_isChecksumValid(const acpi_sdt_t *sdt);
const acpi_sdt_t *acpi_sdt_mapTable(const acpi_sdt_t *sdt_p);
void acpi_sdt_unmapTable(const acpi_sdt_t *sdt);

#endif
