#include <stdbool.h>
#include <stddef.h>

#include "arch/i686/acpi/sdt.h"
#include "arch/i686/mm/vmm.h"

bool acpi_sdt_isChecksumValid(const acpi_sdt_t *sdt);
const acpi_sdt_t *acpi_sdt_mapTable(const acpi_sdt_t *sdt_p);
void acpi_sdt_unmapTable(const acpi_sdt_t *sdt);

bool acpi_sdt_isChecksumValid(const acpi_sdt_t *sdt) {
    uint8_t sum = 0;

    for(size_t i = 0; i < sdt->header.length; i++) {
        sum += ((uint8_t *)sdt)[i];
    }

    return sum == 0;
}

const acpi_sdt_t *acpi_sdt_mapTable(const acpi_sdt_t *sdt_p) {
    size_t offset = ((size_t)sdt_p) & 0xfff;
    const acpi_sdt_t *sdt = (const acpi_sdt_t *)(((size_t)vmm_map(sdt_p, (offset > 0xff8) ? 2 : 1, true)) + offset);

    size_t length = sdt->header.length;

    vmm_unmap(sdt, (offset > 0xff8) ? 2 : 1);

    size_t mappedSize = length + offset;
    size_t mappedPages = (mappedSize + 0xfff) >> 12;

    return (const acpi_sdt_t *)(((size_t)vmm_map(sdt_p, mappedPages, true)) + offset);
}

void acpi_sdt_unmapTable(const acpi_sdt_t *sdt) {
    size_t offset = ((size_t)sdt) & 0xfff;
    size_t mappedSize = sdt->header.length + offset;
    size_t mappedPages = (mappedSize + 0xfff) >> 12;

    vmm_unmap(sdt, mappedPages);
}
