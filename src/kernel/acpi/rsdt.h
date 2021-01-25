#ifndef __KERNEL_ACPI_RSDT_H__
#define __KERNEL_ACPI_RSDT_H__

#include <stdint.h>

#include "acpi/sdt.h"

typedef struct {
    acpi_sdt_header_t header;
    uint32_t pointers[];
} __attribute__((packed)) acpi_rsdt_t;

#endif
