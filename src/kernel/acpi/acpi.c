#include <stdbool.h>
#include <stddef.h>

#include "acpi/fadt.h"
#include "acpi/hpet.h"
#include "acpi/madt.h"
#include "acpi/rsdp.h"
#include "acpi/rsdt.h"
#include "acpi/sdt.h"
#include "acpi/xsdt.h"
#include "libk/stdio.h"
#include "libk/string.h"

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

void acpi_readRsdp(const acpi_rsdp_t *rsdp) {

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

    if(acpi_sdt_isChecksumValid(sdt)) {
        printf("acpi: found table \"%.4s\"\n", sdt->header.signature);

        if(memcmp(sdt->header.signature, "FACP", 4) == 0) {
            printf("acpi: identified FADT\n");

            const acpi_fadt_t *fadt = (const acpi_fadt_t *)sdt;
        } else if(memcmp(sdt->header.signature, "APIC", 4) == 0) {
            printf("acpi: identified MADT\n");

            const acpi_madt_t *madt = (const acpi_madt_t *)sdt;

            acpi_madt_init(madt);
        } else if(memcmp(sdt->header.signature, "HPET", 4) == 0) {
            printf("acpi: identified HPET\n");

            const acpi_hpet_t *hpet = (const acpi_hpet_t *)sdt;
        } else {
            printf("acpi: ignoring table \"%.4s\" with unknown signature\n", sdt->header.signature);
        }
    } else {
        printf("acpi: ignoring table \"%.4s\" with invalid checksum\n", sdt->header.signature);
    }
}
