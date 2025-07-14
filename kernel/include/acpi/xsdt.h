#ifndef __INCLUDE_ACPI_XSDT_H__
#define __INCLUDE_ACPI_XSDT_H__

#include "acpi/acpi.h"
#include "acpi/sdt.h"

struct ts_acpi_xsdt {
    struct ts_acpi_sdtHeader m_header;
    uint64_t m_tablePointers[];
} __attribute__((packed));

int acpi_xsdtParse(struct ts_acpi *p_acpi, const struct ts_acpi_xsdt *p_xsdt);

#endif
