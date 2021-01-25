#ifndef __KERNEL_ACPI_XSDT_H__
#define __KERNEL_ACPI_XSDT_H__

#include <stdint.h>

#include "acpi/sdt.h"

typedef struct {
    acpi_sdt_header_t header;
    uint64_t pointers[];
} __attribute__((packed)) acpi_xsdt_t;

#endif
