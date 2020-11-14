#ifndef __KERNEL_ACPI_RSDT_HPP__
#define __KERNEL_ACPI_RSDT_HPP__

#include <stdint.h>

#include "acpi/acpi.hpp"

namespace kernel {
    typedef struct {
        acpi_sdt_header_t header;
        uint32_t sdt_ptr[];
    } __attribute__((packed)) rsdt_t;
}

#endif
