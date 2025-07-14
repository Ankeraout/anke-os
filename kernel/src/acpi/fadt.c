#include "acpi/fadt.h"
#include "acpi/table.h"

int acpi_fadtInit(struct ts_acpi *p_acpi) {
    const struct ts_acpi_sdt *l_dsdt = NULL;

    if(p_acpi->m_acpi2) {
        l_dsdt = (const struct ts_acpi_sdt *)(uintptr_t)p_acpi->m_fadt->m_xDsdt;
    } else {
        l_dsdt = (const struct ts_acpi_sdt *)(uintptr_t)p_acpi->m_fadt->m_dsdt;
    }

    acpi_tableRegister(p_acpi, &l_dsdt->m_header);

    return 0;
}
