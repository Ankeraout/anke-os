#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "debug.h"
#include "acpi/acpi.h"
#include "acpi/fadt.h"
#include "acpi/rsdp.h"
#include "acpi/rsdt.h"
#include "libk/libk.h"
#include "mm/mm.h"
#include "mm/vmm.h"

size_t rsdp_physicalAddress = 0;
const rsdp_t *rsdp = NULL;
size_t rsdt_physicalAddress = 0;
const rsdt_t *rsdt = NULL;
size_t fadt_physicalAddress = 0;
const fadt_t *fadt = NULL;

bool acpi_sdt_checkChecksum(const acpi_sdt_header_t *acpi_sdt_header) {
    uint8_t checksum = 0;

    for(size_t i = 0; i < acpi_sdt_header->length; i++) {
        checksum += ((const uint8_t *)acpi_sdt_header)[i];
    }

    return checksum == 0;
}

int acpi_init() {
    rsdp = rsdp_find();

    if(!rsdp) {
        return 1;
    }

    char buffer[100];

    kernel_debug("RSDP found and mapped at 0x");
    kernel_debug(hex32((uint32_t)rsdp, buffer));
    kernel_debug("\n");

    // Read RSDT
    rsdt_physicalAddress = rsdp->rsdp1.rsdt_addr;

    // RSDP is no longer needed, we can unmap it
    vmm_free(rsdp);

    kernel_debug("RSDT found at 0x");
    kernel_debug(hex32(rsdt_physicalAddress, buffer));
    kernel_debug("\n");

    // Map RSDT in memory
    rsdt = (const rsdt_t *)acpi_mapSDT((void *)rsdt_physicalAddress);

    kernel_debug("RSDT mapped at 0x");
    kernel_debug(hex32((uint32_t)rsdt, buffer));
    kernel_debug("\n");

    if(!rsdt) {
        return 1;
    }

    // Compute number of entries in RSDT
    size_t rsdt_entryCount = (rsdt->header.length - sizeof(rsdt->header)) / 4;

    kernel_debug("\n");
    kernel_debug(itoa(rsdt_entryCount, buffer, 10));
    kernel_debug(" RSDT entries found:\n");

    for(size_t i = 0; i < rsdt_entryCount; i++) {
        const acpi_sdt_header_t *sdt = (const acpi_sdt_header_t *)acpi_mapSDT((const acpi_sdt_header_t *)rsdt->sdt_ptr[i]);

        kernel_debug("  - ");
        kernel_debug(strncpy(buffer, (const char *)sdt->signature, 4));

        if(acpi_sdt_checkChecksum(sdt)) {
            
        } else {
            kernel_debug(" (invalid checksum)");
        }

        kernel_debug("\n");
    }

    return 0;
}

const acpi_sdt_header_t *acpi_mapSDT(const acpi_sdt_header_t *paddr) {
    size_t offset = ((uint32_t)paddr % MM_PAGE_SIZE);
    size_t mappedSize = MM_PAGE_SIZE - offset;

    size_t pagesToMap = 1;

    // We need to map at least until the "length" field of the header
    if(mappedSize < 8) {
        pagesToMap++;
        mappedSize += MM_PAGE_SIZE;
    }

    // Map the header
    acpi_sdt_header_t *header = (acpi_sdt_header_t *)(vmm_map((void *)paddr, pagesToMap) + offset);

    if(!header) {
        return NULL;
    }

    if(header->length < mappedSize) {
        // All the SDT is already mapped, we can just return the pointer.
        return header;
    } else {
        // The SDT is bigger than the mapped zone, we need to map more.
        size_t size = header->length;

        // Free old space
        vmm_free(header);

        // Allocate bigger space now that we know the size of the SDT
        header = vmm_map(paddr, (size + offset + MM_PAGE_SIZE - 1) / MM_PAGE_SIZE);

        if(!header) {
            return NULL;
        } else {
            return header + offset;
        }
    }
}
