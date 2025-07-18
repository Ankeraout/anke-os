#include "acpi/sdt.h"

uint8_t acpi_sdtComputeChecksum(const struct ts_acpi_sdtHeader *p_sdt) {
    const uint8_t *l_sdt = (const uint8_t *)p_sdt;

    uint8_t l_checksum = 0;

    for(uint32_t l_i = 0; l_i < p_sdt->m_length; l_i++) {
        l_checksum += l_sdt[l_i];
    }

    return l_checksum;
}
