#include "boot/loader/string.h"

#include "boot/loader/acpi/acpi.h"
#include "boot/loader/acpi/rsdt.h"
#include "boot/loader/acpi/xsdt.h"

int acpiInit(struct ts_acpi *p_acpi, const struct ts_acpiRsdp *p_rsdp) {
    // Initialize ACPI structure
    memset(p_acpi, 0, sizeof(struct ts_acpi));

    // Parse RSDP
    acpiRsdpParse(p_acpi, p_rsdp);

    // Parse present tables
    if(p_acpi->m_dsdt != NULL) {
        return acpiDsdtParse(p_acpi);
    }

    return 0;
}
