#ifndef __INCLUDE_ACPI_TABLE_H__
#define __INCLUDE_ACPI_TABLE_H__

#include "acpi/acpi.h"
#include "acpi/sdt.h"

void acpiTableRegister(
    struct ts_acpi *p_acpi,
    const struct ts_acpiSdtHeader *p_table
);

#endif
