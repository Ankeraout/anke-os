#include "acpi/rsdt.h"
#include "acpi/table.h"

int acpi_rsdtParse(struct ts_acpi *p_acpi, const struct ts_acpi_rsdt *p_rsdt) {
    // Validate RSDT
    if(acpi_sdtComputeChecksum(&p_rsdt->m_header) != 0U) {
        return 1;
    }

    p_acpi->m_rsdt = p_rsdt;

    // Parse pointers
    size_t l_dataLength =
        p_rsdt->m_header.m_length - sizeof(struct ts_acpi_sdtHeader);
    size_t l_entryCount = l_dataLength / sizeof(p_rsdt->m_tablePointers[0]);

    for(size_t l_i = 0; l_i < l_entryCount; l_i++) {
        acpi_tableRegister(
            p_acpi,
            (struct ts_acpi_sdtHeader *)(uintptr_t)p_rsdt->m_tablePointers[l_i]
        );
    }

    return 0;
}
