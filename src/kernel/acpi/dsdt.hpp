#ifndef __KERNEL_ACPI_DSDT_HPP__
#define __KERNEL_ACPI_DSDT_HPP__

#include <stdint.h>

#include "acpi/acpi.hpp"

namespace kernel {
    typedef struct {
        acpi_sdt_header_t header;
        uint8_t aml[];
    } __attribute__((packed)) dsdt_t;
}

#endif
