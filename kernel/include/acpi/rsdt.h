#ifndef __INCLUDE_ACPI_RSDT_H__
#define __INCLUDE_ACPI_RSDT_H__

#include "acpi/acpi.h"
#include "acpi/sdt.h"

struct ts_acpiRsdt {
    struct ts_acpiSdtHeader m_header;
    uint32_t m_tablePointers[];
} __attribute__((packed));

int acpiRsdtParse(struct ts_acpi *p_acpi, const struct ts_acpiRsdt *p_rsdt);

#endif
