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
    const struct ts_acpi_rsdp *m_rsdp;
    const struct ts_acpi_fadt *m_fadt;
    const struct ts_acpi_xsdt *m_xsdt;
    const struct ts_acpi_rsdt *m_rsdt;
    const struct ts_acpi_madt *m_madt;
    const struct ts_acpi_sdt *m_dsdt;
    bool m_acpi2;
    struct ts_acpi_node *m_root;
};

int acpi_init(void);
void acpi_setRsdpLocation(const struct ts_acpi_rsdp *p_rsdp);

#endif
