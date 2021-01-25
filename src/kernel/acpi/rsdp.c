#include <stdbool.h>
#include <stdint.h>

#include "acpi/rsdp.h"

bool acpi_rsdp_isChecksumValid(acpi_rsdp_t *rsdp);

bool acpi_rsdp_isChecksumValid(acpi_rsdp_t *rsdp) {
    uint8_t *buffer = (uint8_t *)rsdp;
    uint8_t sum = 0;

    for(int i = 0; i < 20; i++) {
        sum += buffer[i];
    }

    if(sum != 0) {
        return false;
    } else if(rsdp->revision == 0) {
        return true;
    } else if(rsdp->revision == 2) {
        for(int i = 0; i < 16; i++) {
            sum += buffer[i + 20];
        }

        return sum == 0;
    } else {
        return false;
    }

    return true;
}
