#ifndef __MADT_H__
#define __MADT_H__

#include <stdint.h>

#include "acpi/acpi.h"

typedef struct {
    acpi_sdt_header_t header;
    uint32_t localAPIC_addr;
    uint32_t flags;
    uint8_t records[];
} __attribute__((packed)) madt_t;

#endif
