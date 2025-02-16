#ifndef __INCLUDE_ACPI_ACPI_H__
#define __INCLUDE_ACPI_ACPI_H__

#include <stdbool.h>

struct ts_acpi;

#include "acpi/aml.h"
#include "acpi/dsdt.h"
#include "acpi/fadt.h"
#include "acpi/madt.h"
#include "acpi/rsdp.h"
#include "acpi/rsdt.h"
#include "acpi/xsdt.h"

struct ts_acpi {
    const struct ts_acpiRsdp *m_rsdp;
    const struct ts_acpiFadt *m_fadt;
    const struct ts_acpiXsdt *m_xsdt;
    const struct ts_acpiRsdt *m_rsdt;
    const struct ts_acpiMadt *m_madt;
    const struct ts_acpiDsdt *m_dsdt;
    bool m_acpi2;
    struct ts_acpiNode *m_root;
};

int acpiInit(void);
void acpiSetRsdpLocation(const struct ts_acpiRsdp *p_rsdp);

#endif
