#ifndef __KERNEL_ARCH_I686_ACPI_XSDT_H__
#define __KERNEL_ARCH_I686_ACPI_XSDT_H__

#include <stdint.h>

#include "arch/i686/acpi/sdt.h"

typedef struct {
    acpi_sdt_header_t header;
    uint64_t pointers[];
} __attribute__((packed)) acpi_xsdt_t;

#endif
