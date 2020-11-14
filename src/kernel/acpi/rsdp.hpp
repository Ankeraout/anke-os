#ifndef __KERNEL_ACPI_RSDP_HPP__
#define __KERNEL_ACPI_RSDP_HPP__

#include <stdint.h>

namespace kernel {
    typedef struct {
        uint8_t signature[8]; // "RSD PTR "
        uint8_t checksum;
        uint8_t oem_id[6];
        uint8_t revision;
        uint32_t rsdt_addr;
    } __attribute__((packed)) rsdp1_t;

    /**
     * ACPI 2.0 RSDP
     */
    typedef struct {
        rsdp1_t rsdp1;
        uint32_t length;
        uint64_t xsdt_addr;
        uint8_t extendedChecksum;
        uint8_t reserved[3];
    } __attribute__((packed)) rsdp_t;

    /**
     *  Summary:
     *      Tries to locate RSDP.
     * 
     *  Returns:
     *      A pointer to the RSDP structure if it was found, or NULL if it was not
     *      found.
     */
    const rsdp_t *rsdp_find();
}

#endif
