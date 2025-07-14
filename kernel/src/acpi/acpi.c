#include "stdlib.h"
#include "string.h"

#include "acpi/acpi.h"
#include "acpi/rsdt.h"
#include "acpi/xsdt.h"

static const struct ts_acpi_rsdp *s_rsdp = NULL;
static struct ts_acpi s_acpi;

int acpi_init(void) {
    if(s_rsdp == NULL) {
        return -1;
    }

    // Initialize ACPI structure
    memset(&s_acpi, 0, sizeof(struct ts_acpi));

    // Create root scope object
    // TODO

    // Parse RSDP
    acpi_rsdpParse(&s_acpi, s_rsdp);

    // Parse present tables
    if(s_acpi.m_dsdt != NULL) {
        return acpi_dsdtParse(&s_acpi);
    }

    return 0;
}

void acpi_setRsdpLocation(const struct ts_acpi_rsdp *p_rsdp) {
    s_rsdp = p_rsdp;
}
