#ifndef __INCLUDE_ACPI_ACPI_H__
#define __INCLUDE_ACPI_ACPI_H__

#include <stdbool.h>

struct ts_acpi;

#include "boot/loader/acpi/dsdt.h"
#include "boot/loader/acpi/fadt.h"
#include "boot/loader/acpi/madt.h"
#include "boot/loader/acpi/rsdp.h"
#include "boot/loader/acpi/rsdt.h"
#include "boot/loader/acpi/xsdt.h"

struct ts_acpi {
    const struct ts_acpiRsdp *m_rsdp;
    const struct ts_acpiFadt *m_fadt;
    const struct ts_acpiXsdt *m_xsdt;
    const struct ts_acpiRsdt *m_rsdt;
    const struct ts_acpiMadt *m_madt;
    const struct ts_acpiDsdt *m_dsdt;
    bool m_acpi2;
};

int acpiInit(struct ts_acpi *p_acpi, const struct ts_acpiRsdp *p_rsdp);

#endif
