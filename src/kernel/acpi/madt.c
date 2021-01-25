#include <stddef.h>
#include <stdint.h>

#include "acpi/madt.h"
#include "libk/stdio.h"

void acpi_madt_init(const acpi_madt_t *madt);
static void acpi_madt_readRecord_processorLocalApic(const acpi_madt_record_processorLocalApic_t *record);
static void acpi_madt_readRecord_ioApic(const acpi_madt_record_ioApic_t *record);
static void acpi_madt_readRecord_interruptSourceOverride(const acpi_madt_record_interruptSourceOverride_t *record);
static void acpi_madt_readRecord_nonMaskableInterrupts(const acpi_madt_record_nonMaskableInterrupts_t *record);
static void acpi_madt_readRecord_localApicAddressOverride(const acpi_madt_record_localApicAddressOverride_t *record);

void acpi_madt_init(const acpi_madt_t *madt) {
    size_t currentRecordAddress = (size_t)&madt->records;
    size_t endAddress = ((size_t)madt) + madt->header.length;

    printf("acpi: found local APIC at %#08x\n", madt->localApicAddress);

    while(currentRecordAddress < endAddress) {
        const acpi_madt_record_header_t *header = (const acpi_madt_record_header_t *)currentRecordAddress;

        switch(header->type) {
            case ACPI_MADT_RECORDTYPE_PROCESSOR_LOCAL_APIC:
                acpi_madt_readRecord_processorLocalApic((const acpi_madt_record_processorLocalApic_t *)currentRecordAddress);
                break;

            case ACPI_MADT_RECORDTYPE_IO_APIC:
                acpi_madt_readRecord_ioApic((const acpi_madt_record_ioApic_t *)currentRecordAddress);
                break;

            case ACPI_MADT_RECORDTYPE_INTERRUPT_SOURCE_OVERRIDE:
                acpi_madt_readRecord_interruptSourceOverride((const acpi_madt_record_interruptSourceOverride_t *)currentRecordAddress);
                break;

            case ACPI_MADT_RECORDTYPE_NON_MASKABLE_INTERRUPTS:
                acpi_madt_readRecord_nonMaskableInterrupts((const acpi_madt_record_nonMaskableInterrupts_t *)currentRecordAddress);
                break;

            case ACPI_MADT_RECORDTYPE_LOCAL_APIC_ADDRESS_OVERRIDE:
                acpi_madt_readRecord_localApicAddressOverride((const acpi_madt_record_localApicAddressOverride_t *)currentRecordAddress);
                break;
        }

        currentRecordAddress += header->length;
    }
}

static void acpi_madt_readRecord_processorLocalApic(const acpi_madt_record_processorLocalApic_t *record) {
    printf("acpi: found ACPI processor #%x\n", record->processorId);
    printf("acpi: ACPI processor #%x has APIC #%x (", record->processorId, record->apicId);

    if(record->flags & (1 << 0)) {
        printf("enabled");
    } else {
        if(record->flags & (1 << 1)){
            printf("can be enabled");
        } else {
            printf("cannot be enabled");
        }
    }

    printf(")\n");
}

static void acpi_madt_readRecord_ioApic(const acpi_madt_record_ioApic_t *record) {
    printf("acpi: found I/O APIC #%x at %#08x\n", record->ioApicId, record->ioApicAddress);
}

static void acpi_madt_readRecord_interruptSourceOverride(const acpi_madt_record_interruptSourceOverride_t *record) {
    
}

static void acpi_madt_readRecord_nonMaskableInterrupts(const acpi_madt_record_nonMaskableInterrupts_t *record) {
    
}

static void acpi_madt_readRecord_localApicAddressOverride(const acpi_madt_record_localApicAddressOverride_t *record) {
    printf("acpi: overriding local APIC address to %#08x%08x\n", (uint32_t)(record->localApicAddress >> 32), (uint32_t)record->localApicAddress);
}
