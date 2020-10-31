#ifndef __RSDT_H__
#define __RSDT_H__

#include <stdint.h>

#include "acpi/acpi.h"

typedef struct {
    acpi_sdt_header_t header;
    uint32_t sdt_ptr[];
} __attribute__((packed)) rsdt_t;

#endif
