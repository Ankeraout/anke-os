#ifndef __INCLUDE_ACPI_RSDP_H__
#define __INCLUDE_ACPI_RSDP_H__

#include <stddef.h>
#include <stdint.h>

#include "boot/loader/acpi/acpi.h"

struct ts_acpi;

struct ts_acpiRsdp {
    uint8_t m_signature[8];
    uint8_t m_checksum;
    uint8_t m_oemId[6];
    uint8_t m_revision;
    uint32_t m_rsdtAddress;
    uint32_t m_length;
    uint64_t m_xsdtAddress;
    uint8_t m_extendedChecksum;
    uint8_t m_reserved[3];
} __attribute__((packed));

void acpiRsdpParse(struct ts_acpi *p_acpi, const struct ts_acpiRsdp *p_rsdp);
const struct ts_acpiRsdp *acpiRsdpLocate(const void *p_buffer, size_t p_size);

#endif
