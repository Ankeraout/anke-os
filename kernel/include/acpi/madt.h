#ifndef __INCLUDE_ACPI_MADT_H__
#define __INCLUDE_ACPI_MADT_H__

#include "acpi/sdt.h"

struct ts_acpi_madtEntryHeader {
    uint8_t m_entryType;
    uint8_t m_recordLength;
} __attribute__((packed));

struct ts_acpi_madt {
    struct ts_acpi_sdtHeader m_header;
    uint32_t m_localApicAddress;
    uint32_t m_flags;
} __attribute__((packed));

struct ts_acpi_madtEntryLocalApic {
    struct ts_acpi_madtEntryHeader m_header;
    uint8_t m_acpiProcessorId;
    uint8_t m_apicId;
    uint32_t m_flags;
} __attribute__((packed));

struct ts_acpi_madtEntryIoApic {
    struct ts_acpi_madtEntryHeader m_header;
    uint8_t m_ioApicId;
    uint8_t m_reserved;
    uint32_t m_ioApicAddress;
    uint32_t m_globalSystemInterruptBase;
} __attribute__((packed));

struct ts_acpi_madtEntryIoApicInterruptSourceOverride {
    struct ts_acpi_madtEntryHeader m_header;
    uint8_t m_busSource;
    uint8_t m_irqSource;
    uint32_t m_globalSystemInterrupt;
    uint16_t m_base;
} __attribute__((packed));

struct ts_acpi_madtEntryIoApicNmiSource {
    uint8_t m_nmiSource;
    uint8_t m_reserved;
    uint16_t m_flags;
    uint32_t m_globalSystemInterrupt;
} __attribute__((packed));

struct ts_acpi_madtEntryLocalApicNmi {
    uint8_t m_acpiProcessorId;
    uint16_t m_flags;
    uint8_t m_lint;
} __attribute__((packed));

struct ts_acpi_madtEntryLocalApicAddressOverride {
    uint16_t m_reserved;
    uint64_t m_localApicAddress;
} __attribute__((packed));

struct ts_acpi_madtEntryProcessorLocalX2Apic {
    uint16_t m_reserved;
    uint32_t m_x2ApicId;
    uint32_t m_flags;
    uint32_t m_acpiId;
} __attribute__((packed));

#endif
