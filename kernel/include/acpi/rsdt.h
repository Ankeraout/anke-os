#ifndef __INCLUDE_ACPI_RSDT_H__
#define __INCLUDE_ACPI_RSDT_H__

#include "acpi/acpi.h"
#include "acpi/sdt.h"

struct ts_acpi_rsdt {
    struct ts_acpi_sdtHeader m_header;
    uint32_t m_tablePointers[];
} __attribute__((packed));

int acpi_rsdtParse(struct ts_acpi *p_acpi, const struct ts_acpi_rsdt *p_rsdt);

#endif
