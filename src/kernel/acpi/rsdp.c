#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "acpi/rsdp.h"
#include "libk/libk.h"

/**
 *  Summary:
 *      Tries to locate RSDP using EBDA
 * 
 *  Returns:
 *      A pointer to the RSDP structure if it was found, or NULL if it was not
 *      found.
 */
const rsdp_t *rsdp_find_ebda() {
    uint16_t ebdaRealModeSegment = *((uint16_t *)0x0000040e);
    uint32_t ebdaAddress = ebdaRealModeSegment << 4;
    uint8_t *ebda = (uint8_t *)ebdaAddress;
    
    // RSDP is 16-byte aligned, and in the 1st KiB of the EBDA
    for(size_t i = 0; i < 1024; i += 16) {
        if(memcmp(ebda + i, "RSD PTR ", 8) == 0) {
            return (rsdp_t *)ebda + i;
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
    // RSDP is 16-byte aligned
    for(uint8_t *ptr = (uint8_t *)0xe0000; ptr < (uint8_t *)0xfffff; ptr += 16) {
        if(memcmp(ptr, "RSD PTR ", 8) == 0) {
            return (rsdp_t *)ptr;
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
