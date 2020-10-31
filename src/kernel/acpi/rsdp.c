#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "debug.h"
#include "acpi/rsdp.h"
#include "libk/libk.h"
#include "mm/mm.h"
#include "mm/vmm.h"

/**
 *  Summary:
 *      Validates that the RSDP was found.
 * 
 *  Returns:
 *      true if the parameter is a pointer to a valid RSDP, false otherwise.
 */
bool rsdp_validate(const rsdp_t *rsdp) {
    // Check signature
    if(memcmp(&rsdp->rsdp1.signature, "RSD PTR ", 8) != 0) {
        return false;
    }

    // Check checksum
    if(rsdp->rsdp1.revision == 0) {
        // ACPI version 1.0
        uint8_t checksum = 0;
        
        for(size_t i = 0; i < sizeof(rsdp1_t); i++) {
            checksum += ((uint8_t *)rsdp)[i];
        }

        return checksum == 0;
    } else if(rsdp->rsdp1.revision == 2) {
        // ACPI version >= 2.0
        uint8_t checksum = 0;

        for(size_t i = 0; i < sizeof(rsdp_t); i++) {
            checksum += ((uint8_t *)rsdp)[i];
        }

        return checksum == 0;
    } else {
        // Unknown ACPI version
        return false;
    }
}

/**
 *  Summary:
 *      Tries to locate RSDP using EBDA
 * 
 *  Returns:
 *      A pointer to the RSDP structure if it was found, or NULL if it was not
 *      found.
 */
const rsdp_t *rsdp_find_ebda() {
    // Map first 4KiB of RAM
    void *ram = vmm_map((void *)0x00000000, 1);

    if(!ram) {
        return NULL;
    }

    uint16_t ebdaRealModeSegment = *((uint16_t *)(ram + 0x0000040e));
    uint32_t ebdaAddress = ebdaRealModeSegment << 4;

    vmm_free(ram);
    
    // Map first 4 KiB of EBDA
    const uint8_t *ebda = vmm_map((void *)ebdaAddress, 1);

    if(!ebda) {
        return NULL;
    }
    
    // RSDP is 16-byte aligned, and in the 1st KiB of the EBDA
    for(size_t i = 0; i < 1024; i += 16) {
        if(rsdp_validate((const rsdp_t *)(ebda + i))) {
            return (const rsdp_t *)(ebda + i);
        }
    }

    return NULL;
}

/**
 *  Summary:
 *      Tries to locate RSDP in memory region from 0xe0000 to 0xfffff
 * 
 *  Returns:
 *      A pointer to the RSDP structure if it was found, or NULL if it was not
 *      found.
 */
const rsdp_t *rsdp_find_e0000() {
    void *ram = vmm_map((void *)0xe0000, 0x20);

    if(!ram) {
        return NULL;
    }

    // RSDP is 16-byte aligned
    for(size_t i = 0; i < 0x20000; i += 16) {
        if(rsdp_validate((const rsdp_t *)(ram + i))) {
            const size_t rsdpAddress = 0xe0000 + i;
            const size_t rsdpPageOffset = rsdpAddress % MM_PAGE_SIZE;
            const size_t mappedSize = MM_PAGE_SIZE - rsdpPageOffset;
            size_t pagesToMap = 1;

            if(mappedSize < sizeof(rsdp_t)) {
                pagesToMap++;
            }

            vmm_free(ram);
            ram = vmm_map((void *)rsdpAddress, pagesToMap);

            if(!ram) {
                return NULL;
            } else {
                return ram + rsdpPageOffset;
            }
        }
    }

    return NULL;
}

const rsdp_t *rsdp_find() {
    // Try locating RSDP using EBDA
    const rsdp_t *rsdp = rsdp_find_ebda();

    if(rsdp) {
        return rsdp;
    } else {
        // Try locating RSDP using memory region from 0xe0000 to 0xfffff
        return rsdp_find_e0000();
    }
}
