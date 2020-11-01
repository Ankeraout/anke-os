#ifndef __DSDT_H__
#define __DSDT_H__

#include <stdint.h>

#include "acpi/acpi.h"

typedef struct {
    acpi_sdt_header_t header;
    uint8_t aml[];
} __attribute__((packed)) dsdt_t;

#endif
