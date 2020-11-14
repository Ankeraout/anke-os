#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "debug.hpp"
#include "acpi/acpi.hpp"
#include "acpi/dsdt.hpp"
#include "acpi/fadt.hpp"
#include "acpi/madt.hpp"
#include "acpi/rsdp.hpp"
#include "acpi/rsdt.hpp"
#include "libk/libk.hpp"
#include "mm/mm.hpp"
#include "mm/vmm.hpp"

namespace kernel {
    size_t rsdp_physicalAddress = 0;
    const rsdp_t *rsdp = NULL;
    size_t rsdt_physicalAddress = 0;
    const rsdt_t *rsdt = NULL;
    size_t fadt_physicalAddress = 0;
    const fadt_t *fadt = NULL;
    size_t madt_physicalAddress = 0;
    const madt_t *madt = NULL;
    size_t dsdt_physicalAddress = 0;
    const dsdt_t *dsdt = NULL;
    size_t ssdt_physicalAddress = 0;
    const dsdt_t *ssdt = NULL;

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

        debug("RSDP found and mapped at 0x");
        debug(std::hex32((uint32_t)rsdp, buffer));
        debug("\n");

        // Read RSDT
        rsdt_physicalAddress = rsdp->rsdp1.rsdt_addr;

        // RSDP is no longer needed, we can unmap it
        vmm_free(rsdp);

        debug("RSDT found at 0x");
        debug(std::hex32(rsdt_physicalAddress, buffer));
        debug("\n");

        // Map RSDT in memory
        rsdt = (const rsdt_t *)acpi_mapSDT((acpi_sdt_header_t *)rsdt_physicalAddress);

        debug("RSDT mapped at 0x");
        debug(std::hex32((uint32_t)rsdt, buffer));
        debug("\n");

        if(!rsdt) {
            return 1;
        }

        // Compute number of entries in RSDT
        size_t rsdt_entryCount = (rsdt->header.length - sizeof(rsdt->header)) / 4;

        debug("\n");
        debug(std::itoa(rsdt_entryCount, buffer, 10));
        debug(" RSDT entries found:\n");

        for(size_t i = 0; i < rsdt_entryCount; i++) {
            const acpi_sdt_header_t *sdt = (const acpi_sdt_header_t *)acpi_mapSDT((const acpi_sdt_header_t *)rsdt->sdt_ptr[i]);

            debug("  - ");
            debug(std::strncpy(buffer, (const char *)sdt->signature, 4));

            if(acpi_sdt_checkChecksum(sdt)) {
                if(std::strncmp("FACP", (const char *)sdt->signature, 4) == 0) {
                    fadt = (const fadt_t *)sdt;
                    fadt_physicalAddress = rsdt->sdt_ptr[i];
                    dsdt_physicalAddress = fadt->dsdtPtr;
                    dsdt = (const dsdt_t *)acpi_mapSDT((const acpi_sdt_header_t *)dsdt_physicalAddress);

                    if(dsdt && !acpi_sdt_checkChecksum((const acpi_sdt_header_t *)dsdt)) {
                        vmm_unmap(dsdt);
                    }
                } else if(std::strncmp("APIC", (const char *)sdt->signature, 4) == 0) {
                    madt = (const madt_t *)sdt;
                    madt_physicalAddress = rsdt->sdt_ptr[i];
                }
            } else {
                debug(" (invalid checksum)");
            }

            debug("\n");
        }

        if(fadt) {
            debug("FADT was found\n");
        }

        if(madt) {
            debug("MADT was found\n");
        }

        if(dsdt) {
            debug("DSDT was found\n");
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
        acpi_sdt_header_t *header = (acpi_sdt_header_t *)vmm_map((void *)paddr, pagesToMap, true);

        if(!header) {
            return NULL;
        } else {
            header = (acpi_sdt_header_t *)(((size_t)header) + offset);
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
            header = (acpi_sdt_header_t *)vmm_map(paddr, (size + offset + MM_PAGE_SIZE - 1) / MM_PAGE_SIZE, true);

            if(!header) {
                return NULL;
            } else {
                return (acpi_sdt_header_t *)(((size_t)header) + offset);
            }
        }
    }
}
