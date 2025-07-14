#include "string.h"
#include "acpi/rsdp.h"

static uint8_t acpi_rsdpComputeChecksum(const struct ts_acpi_rsdp *p_rsdp);
static uint8_t acpi_rsdpComputeExtendedChecksum(
    const struct ts_acpi_rsdp *p_rsdp
);

void acpi_rsdpParse(struct ts_acpi *p_acpi, const struct ts_acpi_rsdp *p_rsdp) {
    // Validate RSDP
    if(acpi_rsdpComputeChecksum(p_rsdp) != 0U) {
        // TODO: Error: RSDP checksum error
        return;
    }

    p_acpi->m_rsdp = p_rsdp;
    p_acpi->m_acpi2 = p_rsdp->m_revision != 0U;

    // If ACPI version is 2.0, check the extended checksum
    if(p_acpi->m_acpi2) {
        p_acpi->m_acpi2 &= acpi_rsdpComputeExtendedChecksum(p_rsdp) == 0U;
    }

    if(p_acpi->m_acpi2) {
        acpi_xsdtParse(
            p_acpi,
            (const struct ts_acpi_xsdt *)((uintptr_t)p_rsdp->m_xsdtAddress)
        );
    } else {
        acpi_rsdtParse(
            p_acpi,
            (const struct ts_acpi_rsdt *)((uintptr_t)p_rsdp->m_rsdtAddress)
        );
    }
}

const struct ts_acpi_rsdp *acpi_rsdpLocate(const void *p_buffer, size_t p_size) {
    const uint8_t *l_buffer = (const uint8_t *)p_buffer;
    size_t l_offset = 0;

    while(l_offset < p_size) {
        const struct ts_acpi_rsdp *l_rsdp =
            (const struct ts_acpi_rsdp *)&l_buffer[l_offset];

        if(memcmp(l_rsdp->m_signature, "RSD PTR ", 8U) == 0) {
            return l_rsdp;
        }

        l_offset += 16U;
    }

    return NULL;
}

static uint8_t acpi_rsdpComputeChecksum(const struct ts_acpi_rsdp *p_rsdp) {
    const uint8_t *l_rsdp = (const uint8_t *)p_rsdp;

    uint8_t l_checksum = 0;

    for(uint32_t l_i = 0; l_i < 20U; l_i++) {
        l_checksum += l_rsdp[l_i];
    }

    return l_checksum;
}

static uint8_t acpi_rsdpComputeExtendedChecksum(
    const struct ts_acpi_rsdp *p_rsdp
) {
    const uint8_t *l_rsdp = (const uint8_t *)p_rsdp;

    uint8_t l_checksum = 0;

    for(uint32_t l_i = 0; l_i < sizeof(*p_rsdp); l_i++) {
        l_checksum += l_rsdp[l_i];
    }

    return l_checksum;
}
