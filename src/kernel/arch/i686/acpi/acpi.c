#include <stdbool.h>

#include "arch/i686/acpi/rsdp.h"
#include "arch/i686/acpi/rsdt.h"
#include "arch/i686/acpi/sdt.h"
#include "arch/i686/acpi/xsdt.h"
#include "libk/stdio.h"

bool acpi_supported = true;
bool acpi_initialized = false;
int acpi_version_major = 0;
int acpi_version_minor = 0;

void acpi_init();
static void acpi_readRsdt(const acpi_rsdt_t *rsdt);
static void acpi_readXsdt(const acpi_xsdt_t *xsdt);
static void acpi_addSdt(const acpi_sdt_t *sdt_p);

void acpi_init() {
    const acpi_rsdp_t *rsdp = acpi_rsdp_locate();

    if(!rsdp) {
        printf("acpi: could not locate RSDP\n");
        acpi_supported = false;
    } else {
        printf("acpi: located RSDP\n");

        if(rsdp->revision == 0) {
            printf("acpi: detected ACPI version 1.0\n");
            acpi_readRsdt((const acpi_rsdt_t *)acpi_sdt_mapTable((const acpi_sdt_t *)rsdp->rsdtPtr));
        } else if(rsdp->revision == 2) {
            printf("acpi: detected ACPI version >= 2.0\n");
            acpi_readXsdt((const acpi_xsdt_t *)acpi_sdt_mapTable((const acpi_sdt_t *)(size_t)rsdp->xsdtPtr));
        } else {
            printf("acpi: Unknown ACPI version\n");
        }
    }
}

static void acpi_readRsdt(const acpi_rsdt_t *rsdt) {
    size_t entriesCount = (rsdt->header.length - sizeof(acpi_sdt_header_t)) >> 2;

    for(size_t i = 0; i < entriesCount; i++) {
        acpi_addSdt((const acpi_sdt_t *)rsdt->pointers[i]);
    }
}

static void acpi_readXsdt(const acpi_xsdt_t *xsdt) {
    size_t entriesCount = (xsdt->header.length - sizeof(acpi_sdt_header_t)) >> 3;

    for(size_t i = 0; i < entriesCount; i++) {
        acpi_addSdt((const acpi_sdt_t *)(size_t)xsdt->pointers[i]);
    }
}

static void acpi_addSdt(const acpi_sdt_t *sdt_p) {
    const acpi_sdt_t *sdt = acpi_sdt_mapTable(sdt_p);

    printf("acpi: found table with signature \"%.4s\"\n", sdt->header.signature);

    if(acpi_sdt_isChecksumValid(sdt)) {
        printf("acpi: checksum of table is valid\n");
    } else {
        printf("acpi: checksum of table is invalid\n");
    }
}
