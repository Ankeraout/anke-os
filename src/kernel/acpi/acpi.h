#ifndef __ACPI_H__
#define __ACPI_H__

#include <stdint.h>

typedef struct {
    uint8_t signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) acpi_sdt_header_t;

/**
 *  Summary:
 *      Checks the checksum of the given ACPI SDT table.
 * 
 *  Returns:
 *      0 if the checksum is correct, any other value otherwise.
 */
int acpi_sdt_checkChecksum(const acpi_sdt_header_t *acpi_sdt_header);

/**
 *  Summary:
 *      Initializes ACPI on the system.
 * 
 *  Returns:
 *      0 if the initialization was successful (system is in ACPI mode), or any
 *      other value if an error occurred while initializing ACPI (unsupported
 *      by the system or initialization failed).
 */
int acpi_init();

/**
 *  Summary:
 *      Maps a SDT in RAM
 * 
 *  Args:
 *      - paddr: The physical address of the SDT to map
 * 
 *  Returns:
 *      0 if the mapping was successful, or any other value if an error
 *      occurred.
 * 
 *  Error cases:
 *      - vmm_map() failed
 */
const acpi_sdt_header_t *acpi_mapSDT(const acpi_sdt_header_t *paddr);

#endif
