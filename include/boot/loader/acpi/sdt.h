#ifndef __INCLUDE_ACPI_SDT_H__
#define __INCLUDE_ACPI_SDT_H__

#include <stdint.h>

struct ts_acpiSdtHeader {
    uint8_t m_signature[4];
    uint32_t m_length;
    uint8_t m_revision;
    uint8_t m_checksum;
    uint8_t m_oemId[6];
    uint8_t m_oemTableId[8];
    uint32_t m_oemRevision;
    uint32_t m_creatorId;
    uint32_t m_creatorRevision;
} __attribute__((packed));

uint8_t acpiSdtComputeChecksum(const struct ts_acpiSdtHeader *p_sdt);

#endif
