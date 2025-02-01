#include "printk.h"
#include "string.h"

#include "acpi/table.h"

void acpiTableRegister(
    struct ts_acpi *p_acpi,
    const struct ts_acpiSdtHeader *p_table
) {
    // Make sure that the checksum corresponds
    if(acpiSdtComputeChecksum(p_table) != 0U) {
        return;
    }

    char l_tableName[5] = {
        p_table->m_signature[0],
        p_table->m_signature[1],
        p_table->m_signature[2],
        p_table->m_signature[3],
        0
    };

    printk("acpi: Table %s", l_tableName);
    printk(" at %p\n", p_table);

    // Check table signature
    if(memcmp(p_table->m_signature, "FACP", 4U) == 0) {
        if(
            (p_acpi->m_fadt != NULL)
            && (p_acpi->m_fadt != (const struct ts_acpiFadt *)p_table)
        ) {
            // TODO: Error: multiple FADT
            return;
        }

        p_acpi->m_fadt = (struct ts_acpiFadt *)p_table;

        acpiFadtInit(p_acpi);
    } else if(memcmp(p_table->m_signature, "APIC", 4) == 0) {
        if(
            (p_acpi->m_madt != NULL)
            && (p_acpi->m_madt != (const struct ts_acpiMadt *)p_table)
        ) {
            // TODO: Error: multiple MADT
            return;
        }

        p_acpi->m_madt = (struct ts_acpiMadt *)p_table;
    } else if(memcmp(p_table->m_signature, "RSDT", 4) == 0) {
        if(
            (p_acpi->m_rsdt != NULL)
            && (p_acpi->m_rsdt != (const struct ts_acpiRsdt *)p_table)
        ) {
            // TODO: Error: multiple RSDT
            return;
        }

        p_acpi->m_rsdt = (struct ts_acpiRsdt *)p_table;
    } else if(memcmp(p_table->m_signature, "XSDT", 4U) == 0) {
        if(
            (p_acpi->m_xsdt != NULL)
            && (p_acpi->m_xsdt != (const struct ts_acpiXsdt *)p_table)
        ) {
            // TODO: Error: multiple RSDT
            return;
        }

        p_acpi->m_xsdt = (struct ts_acpiXsdt *)p_table;
    } else if(memcmp(p_table->m_signature, "DSDT", 4U) == 0) {
        if(
            (p_acpi->m_dsdt != NULL)
            && (p_acpi->m_dsdt != (const struct ts_acpiDsdt *)p_table)
        ) {
            // TODO: Error: multiple DSDT
        }

        p_acpi->m_dsdt = (const struct ts_acpiDsdt *)p_table;
    } else {
        // TODO: unknown table
    }
}
