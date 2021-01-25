#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "acpi/sdt.h"

bool acpi_sdt_isChecksumValid(const acpi_sdt_t *sdt);

bool acpi_sdt_isChecksumValid(const acpi_sdt_t *sdt) {
    uint8_t sum = 0;

    for(size_t i = 0; i < sdt->header.length; i++) {
        sum += ((uint8_t *)sdt)[i];
    }

    return sum == 0;
}
