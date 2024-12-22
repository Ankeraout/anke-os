#ifndef __INCLUDE_ACPI_MADT_H__
#define __INCLUDE_ACPI_MADT_H__

#include "boot/loader/acpi/sdt.h"

struct ts_acpiMadtEntryHeader {
    uint8_t m_entryType;
    uint8_t m_recordLength;
} __attribute__((packed));

struct ts_acpiMadt {
    struct ts_acpiSdtHeader m_header;
    uint32_t m_localApicAddress;
    uint32_t m_flags;
} __attribute__((packed));

struct ts_acpiMadtEntryLocalApic {
    struct ts_acpiMadtEntryHeader m_header;
    uint8_t m_acpiProcessorId;
    uint8_t m_apicId;
    uint32_t m_flags;
} __attribute__((packed));

struct ts_acpiMadtEntryIoApic {
    struct ts_acpiMadtEntryHeader m_header;
    uint8_t m_ioApicId;
    uint8_t m_reserved;
    uint32_t m_ioApicAddress;
    uint32_t m_globalSystemInterruptBase;
} __attribute__((packed));

struct ts_acpiMadtEntryIoApicInterruptSourceOverride {
    struct ts_acpiMadtEntryHeader m_header;
    uint8_t m_busSource;
    uint8_t m_irqSource;
    uint32_t m_globalSystemInterrupt;
    uint16_t m_base;
} __attribute__((packed));

struct ts_acpiMadtEntryIoApicNmiSource {
    uint8_t m_nmiSource;
    uint8_t m_reserved;
    uint16_t m_flags;
    uint32_t m_globalSystemInterrupt;
} __attribute__((packed));

struct ts_acpiMadtEntryLocalApicNmi {
    uint8_t m_acpiProcessorId;
    uint16_t m_flags;
    uint8_t m_lint;
} __attribute__((packed));

struct ts_acpiMadtEntryLocalApicAddressOverride {
    uint16_t m_reserved;
    uint64_t m_localApicAddress;
} __attribute__((packed));

struct ts_acpiMadtEntryProcessorLocalX2Apic {
    uint16_t m_reserved;
    uint32_t m_x2ApicId;
    uint32_t m_flags;
    uint32_t m_acpiId;
} __attribute__((packed));

#endif
