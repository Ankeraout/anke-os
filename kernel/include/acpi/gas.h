#ifndef __INCLUDE_ACPI_GAS_H__
#define __INCLUDE_ACPI_GAS_H__

#include <stdint.h>

struct ts_acpi_genericAddressStructure {
    uint8_t m_addressSpace;
    uint8_t m_bitWidth;
    uint8_t m_bitOffset;
    uint8_t m_accessSize;
    uint64_t m_address;
} __attribute__((packed));

#endif
