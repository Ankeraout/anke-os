#include "acpi/dsdt.h"

int acpiDsdtParse(struct ts_acpi *p_acpi) {
    // Validate RSDT
    if(acpiSdtComputeChecksum(&p_acpi->m_dsdt->m_header) != 0U) {
        return 1;
    }

    return 0;
}
