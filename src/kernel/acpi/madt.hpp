#ifndef __KERNEL_ACPI_MADT_HPP__
#define __KERNEL_ACPI_MADT_HPP__

#include <stdint.h>

#include "acpi/acpi.hpp"

namespace kernel {
    typedef struct {
        acpi_sdt_header_t header;
        uint32_t localAPIC_addr;
        uint32_t flags;
        uint8_t records[];
    } __attribute__((packed)) madt_t;
}

#endif
