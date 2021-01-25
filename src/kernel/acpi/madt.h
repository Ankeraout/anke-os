#ifndef __KERNEL_ACPI_MADT_H__
#define __KERNEL_ACPI_MADT_H__

#include <stdint.h>

#include "acpi/sdt.h"

enum {
    ACPI_MADT_RECORDTYPE_PROCESSOR_LOCAL_APIC,
    ACPI_MADT_RECORDTYPE_IO_APIC,
    ACPI_MADT_RECORDTYPE_INTERRUPT_SOURCE_OVERRIDE,
    ACPI_MADT_RECORDTYPE_NON_MASKABLE_INTERRUPTS = 4,
    ACPI_MADT_RECORDTYPE_LOCAL_APIC_ADDRESS_OVERRIDE
};

typedef struct {
    acpi_sdt_header_t header;
    uint32_t localApicAddress;
    uint32_t flags;
    uint8_t records[];
} __attribute__((packed)) acpi_madt_t;

typedef struct {
    uint8_t type;
    uint8_t length;
} __attribute__((packed)) acpi_madt_record_header_t;

typedef struct {
    acpi_madt_record_header_t header;
    uint8_t processorId;
    uint8_t apicId;
    uint32_t flags;
} __attribute__((packed)) acpi_madt_record_processorLocalApic_t;

typedef struct {
    acpi_madt_record_header_t header;
    uint8_t ioApicId;
    uint8_t reserved;
    uint32_t ioApicAddress;
    uint32_t globalSystemInterruptBase;
} __attribute__((packed)) acpi_madt_record_ioApic_t;

typedef struct {
    acpi_madt_record_header_t header;
    uint8_t busSource;
    uint8_t irqSource;
    uint32_t globalSystemInterrupt;
    uint16_t flags;
} __attribute__((packed)) acpi_madt_record_interruptSourceOverride_t;

typedef struct {
    acpi_madt_record_header_t header;
    uint8_t processorId;
    uint16_t flags;
    uint8_t lintNumber;
} __attribute__((packed)) acpi_madt_record_nonMaskableInterrupts_t;

typedef struct {
    acpi_madt_record_header_t header;
    uint16_t reserved;
    uint64_t localApicAddress;
} __attribute__((packed)) acpi_madt_record_localApicAddressOverride_t;

void acpi_madt_init(const acpi_madt_t *madt);

#endif
