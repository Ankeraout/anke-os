#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/i686/acpi/rsdp.h"
#include "arch/i686/mm/vmm.h"

#include "libk/string.h"

const acpi_rsdp_t *acpi_rsdp_locate();
void acpi_rsdp_unmap();
static const acpi_rsdp_t *acpi_rsdp_locate_ebda();
static const acpi_rsdp_t *acpi_rsdp_locate_bios();
static bool acpi_rsdp_isChecksumValid(acpi_rsdp_t *rsdp);

static int foundMethod;

static union {
    struct {
        const void *base;
        size_t size_pages;
    } ebda;

    const void *bios;
} rsdp_data;

const acpi_rsdp_t *acpi_rsdp_locate() {
    const acpi_rsdp_t *acpi_rsdp = acpi_rsdp_locate_ebda();

    if(acpi_rsdp) {
        return acpi_rsdp;
    } else {
        return acpi_rsdp_locate_bios();
    }
}

static const acpi_rsdp_t *acpi_rsdp_locate_ebda() {
    // Map BDA to memory
    const void *bda = vmm_map(0x00000000, 1, true);

    // Read pointer to EBDA
    const void *ebda_ptr = (const void *)(*((uint16_t *)(((size_t)bda) + 0x40e)) << 4);

    // Unmap BDA
    vmm_unmap(bda, 1);

    // Compute EBDA size
    size_t ebda_size = 0xa0000 - (size_t)ebda_ptr;
    rsdp_data.ebda.size_pages = (ebda_size + 0xfff) >> 12;

    // Map EBDA
    rsdp_data.ebda.base = vmm_map(ebda_ptr, rsdp_data.ebda.size_pages, true);
    const void *ebda = (const void *)(((size_t)rsdp_data.ebda.base) + (((size_t)ebda_ptr) & 0xfff));

    // Look for RSDP signature
    for(size_t i = 0; i < ebda_size; i += 16) {
        if(memcmp((const char *)(((size_t)ebda) + i), "RSD PTR ", 8) == 0) {
            acpi_rsdp_t *rsdp = (acpi_rsdp_t *)(((size_t)ebda) + i);
            
            if(acpi_rsdp_isChecksumValid(rsdp)) {
                foundMethod = 0;

                return rsdp;
            }
        }
    }

    // RSDP was not found in EBDA, unmap it
    vmm_unmap(rsdp_data.ebda.base, rsdp_data.ebda.size_pages);

    return NULL;
}

static const acpi_rsdp_t *acpi_rsdp_locate_bios() {
    // Map BIOS area
    rsdp_data.bios = vmm_map((const void *)0xe0000, 32, true);

    // Look for RSDP signature
    for(size_t i = 0; i < 0x20000; i += 16) {
        if(memcmp((const char *)(((size_t)rsdp_data.bios) + i), "RSD PTR ", 8) == 0) {
            acpi_rsdp_t *rsdp = (acpi_rsdp_t *)(((size_t)rsdp_data.bios) + i);
            
            if(acpi_rsdp_isChecksumValid(rsdp)) {
                foundMethod = 1;

                return rsdp;
            }
        }
    }

    // RSDP was not found in BIOS, unmap it
    vmm_unmap(rsdp_data.bios, 32);

    return NULL;
}

static bool acpi_rsdp_isChecksumValid(acpi_rsdp_t *rsdp) {
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

void acpi_rsdp_unmap() {
    if(foundMethod == 0) {
        vmm_unmap(rsdp_data.ebda.base, rsdp_data.ebda.size_pages);
    } else if(foundMethod == 1) {
        vmm_unmap(rsdp_data.bios, 32);
    }
}
