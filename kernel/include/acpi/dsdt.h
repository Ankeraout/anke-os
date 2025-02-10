#ifndef __INCLUDE_ACPI_DSDT_H__
#define __INCLUDE_ACPI_DSDT_H__

#include "acpi/acpi.h"
#include "acpi/sdt.h"

struct ts_acpiDsdt {
    struct ts_acpiSdtHeader m_header;
    uint8_t m_data[];
} __attribute__((packed));

int acpiDsdtParse(struct ts_acpi *p_acpi);

#endif
