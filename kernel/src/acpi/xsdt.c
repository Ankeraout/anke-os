#include <stddef.h>

#include "acpi/table.h"
#include "acpi/xsdt.h"

int acpi_xsdtParse(struct ts_acpi *p_acpi, const struct ts_acpi_xsdt *p_xsdt) {
    // Validate RSDT
    if(acpi_sdtComputeChecksum(&p_xsdt->m_header) != 0U) {
        return 1;
    }

    p_acpi->m_xsdt = p_xsdt;

    // Parse pointers
    size_t l_dataLength =
        p_xsdt->m_header.m_length - sizeof(struct ts_acpi_sdtHeader);
    size_t l_entryCount = l_dataLength / sizeof(p_xsdt->m_tablePointers[0]);

    for(size_t l_i = 0; l_i < l_entryCount; l_i++) {
        acpi_tableRegister(
            p_acpi,
            (struct ts_acpi_sdtHeader *)(uintptr_t)p_xsdt->m_tablePointers[l_i]
        );
    }

    return 0;
}
