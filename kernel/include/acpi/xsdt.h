#ifndef __INCLUDE_ACPI_XSDT_H__
#define __INCLUDE_ACPI_XSDT_H__

#include "acpi/acpi.h"
#include "acpi/sdt.h"

struct ts_acpi;

struct ts_acpiXsdt {
    struct ts_acpiSdtHeader m_header;
    uint64_t m_tablePointers[];
} __attribute__((packed));

int acpiXsdtParse(struct ts_acpi *p_acpi, const struct ts_acpiXsdt *p_xsdt);

#endif
