#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/x86/inline.h"
#include "dev/acpi.h"
#include "klibc/string.h"
#include "debug.h"

struct ts_acpiRsdp {
    uint8_t a_signature[8];
    uint8_t a_checksum;
    uint8_t a_oem[6];
    uint8_t a_revision;
    uint32_t a_rsdtAddress;
} __attribute__((packed));

struct ts_acpiRsdp2 {
    struct ts_acpiRsdp rsdp1;
    uint32_t a_length;
    uint64_t a_xsdtAddress;
    uint8_t a_extendedChecksum;
    uint8_t a_reserved[3];
} __attribute__((packed));

struct ts_acpiSdtHeader {
    uint8_t a_signature[4];
    uint32_t a_length;
    uint8_t a_revision;
    uint8_t a_checksum;
    uint8_t a_oemId[6];
    uint8_t a_oemTableId[8];
    uint32_t a_oemRevision;
    uint32_t a_creatorId;
    uint32_t a_creatorRevision;
} __attribute__((packed));

struct ts_acpiRsdt {
    struct ts_acpiSdtHeader a_header;
    uint32_t a_otherTables[];
} __attribute__((packed));

struct ts_acpiXsdt {
    struct ts_acpiSdtHeader a_header;
    uint64_t a_otherTables[];
} __attribute__((packed));

struct ts_acpiGeneralAddressStructure {
    uint8_t a_addressSpace;
    uint8_t a_bitWidth;
    uint8_t a_bitOffset;
    uint8_t a_accessSize;
    uint64_t a_address;
} __attribute__((packed));

struct ts_acpiFadt {
    struct ts_acpiSdtHeader a_header;
    uint32_t a_firmwareCtrl;
    uint32_t a_dsdt;
    uint8_t a_reserved;
    uint8_t a_preferredPowerManagementProfile;
    uint16_t a_sciInterrupt;
    uint32_t a_smiCommandPort;
    uint8_t a_acpiEnable;
    uint8_t a_acpiDisable;
    uint8_t a_s4BiosReq;
    uint8_t a_pStateControl;
    uint32_t a_pm1aEventBlock;
    uint32_t a_pm1bEventBlock;
    uint32_t a_pm1aControlBlock;
    uint32_t a_pm1bControlBlock;
    uint32_t a_pm2ControlBlock;
    uint32_t a_pmTimerBlock;
    uint32_t a_gpe0Block;
    uint32_t a_gpe1Block;
    uint8_t a_pm1EventLength;
    uint8_t a_pm1ControlLength;
    uint8_t a_pm2ControlLength;
    uint8_t a_pmTimerLength;
    uint8_t a_gpe0Length;
    uint8_t a_gpe1Length;
    uint8_t a_gpe1Base;
    uint8_t a_cStateControl;
    uint16_t a_worstC2Latency;
    uint16_t a_worstC3Latency;
    uint16_t a_flushSize;
    uint16_t a_flushStride;
    uint8_t a_dutyOffset;
    uint8_t a_dutyWidth;
    uint8_t a_dayAlarm;
    uint8_t a_monthAlarm;
    uint8_t a_century;
    uint16_t a_bootArchitectureFlags;
    uint8_t a_reserved2;
    uint32_t a_flags;
    struct ts_acpiGeneralAddressStructure a_resetRegister;
    uint8_t a_resetValue;
    uint8_t a_reserved3[3];
    uint64_t a_xFirmwareControl;
    uint64_t a_xDsdt;
    struct ts_acpiGeneralAddressStructure a_pm1aEventBlockAddress;
    struct ts_acpiGeneralAddressStructure a_pm1bEventBlockAddress;
    struct ts_acpiGeneralAddressStructure a_pm1aControlBlockAddress;
    struct ts_acpiGeneralAddressStructure a_pm1bControlBlockAddress;
    struct ts_acpiGeneralAddressStructure a_pm2ControlBlockAddress;
    struct ts_acpiGeneralAddressStructure a_pmTimerBlockAddress;
    struct ts_acpiGeneralAddressStructure a_gpe0BlockAddress;
    struct ts_acpiGeneralAddressStructure a_gpe1BlockAddress;
} __attribute__((packed));

static void acpiFindRsdp(void);
static void acpiFindRsdpAt(const void *p_ptr, size_t p_regionSize);
static bool acpiIsSdtUsable(const struct ts_acpiSdtHeader *p_table);
static uint8_t acpiComputeChecksum(const void *p_table, size_t p_length);
static void acpiExploreRsdt(const struct ts_acpiRsdt *p_rsdt);
static void acpiExploreXsdt(const struct ts_acpiXsdt *p_xsdt);
static void acpiExploreTable(const struct ts_acpiSdtHeader *p_sdt);

static const char s_rsdpSignature[8] = "RSD PTR ";
static const size_t s_rsdpSignatureLength = 8;
static const struct ts_acpiRsdp2 *s_acpiRsdp = NULL;
static int s_acpiRsdpVersion = 0;
static const struct ts_acpiFadt *s_acpiFadt = NULL;

void acpiInit(void) {
    acpiFindRsdp();

    if(s_acpiRsdp == NULL) {
        // No ACPI
        debugPrint("acpi: RSDP was not found.\n");
        return;
    }

    debugPrint("acpi: ACPI RSDP was found at ");
    debugPrintPointer(s_acpiRsdp);
    debugPrint(".\n");

    if(s_acpiRsdpVersion == 1) {
        debugPrint("acpi: ACPI RSDP 1.0 structure found.\n");
        acpiExploreRsdt((const struct ts_acpiRsdt *)(uintptr_t)s_acpiRsdp->rsdp1.a_rsdtAddress);
    } else if(s_acpiRsdpVersion == 2) {
        debugPrint("acpi: ACPI RSDP 2.0 structure found.\n");
        acpiExploreXsdt((const struct ts_acpiXsdt *)(uintptr_t)s_acpiRsdp->a_xsdtAddress);
    } else {
        debugPrint("acpi: Unknown ACPI RSDP version found.\n");
    }
}

bool acpiIsPs2ControllerPresent(void) {
    if(s_acpiRsdp == NULL) {
        // The system does not support ACPI: PS/2 controller is present.
        return true;
    }

    if(s_acpiFadt == NULL) {
        // The FADT table was not found: PS/2 controller is present.
        return true;
    }

    if(s_acpiRsdp->rsdp1.a_revision == 0) {
        // ACPI 1.0 => boot architecture flags field is invalid.
        return true;
    }

    return (s_acpiFadt->a_bootArchitectureFlags & (1 << 1)) != 0;
}

static void acpiFindRsdp(void) {
    acpiFindRsdpAt(
        (const void *)0x00000000000e0000,
        0x20000
    );
}

static void acpiFindRsdpAt(
    const void *p_ptr,
    size_t p_regionSize
) {
    for(size_t l_index = 0; l_index < p_regionSize; l_index += 16) {
        const struct ts_acpiRsdp2 *l_castedPtr =
            (const struct ts_acpiRsdp2 *)&(((const uint8_t *)p_ptr)[l_index]);

        if(memcmp(l_castedPtr, s_rsdpSignature, s_rsdpSignatureLength) == 0) {
            // Compute checksum
            if(acpiComputeChecksum(l_castedPtr, sizeof(struct ts_acpiRsdp)) != 0) {
                continue;
            }

            // Valid ACPI RSDP 1.0 structure found.
            s_acpiRsdpVersion = 1;
            s_acpiRsdp = (const struct ts_acpiRsdp2 *)l_castedPtr;

            if(s_acpiRsdp->rsdp1.a_revision == 0) {
                // ACPI version is 1.0, we do not need to check for ACPI 2.0
                // structure fields.
                return;
            }

            // ACPI version is 2.0+, so we check for ACPI RSDP 2.0 structure
            // fields.
            if(acpiComputeChecksum(l_castedPtr, sizeof(struct ts_acpiRsdp2)) != 0) {
                s_acpiRsdpVersion = 1;
            } else {
                s_acpiRsdpVersion = 2;
            }
        }
    }
}

static bool acpiIsSdtUsable(const struct ts_acpiSdtHeader *p_table) {
    return (acpiComputeChecksum(p_table, p_table->a_length) == 0);
}

static uint8_t acpiComputeChecksum(const void *p_table, size_t p_length) {
    uint8_t l_checksum = 0;

    for(size_t l_index = 0; l_index < p_length; l_index++) {
        l_checksum += ((const uint8_t *)p_table)[l_index];
    }

    return l_checksum;
}

static void acpiExploreRsdt(const struct ts_acpiRsdt *p_rsdt) {
    if(!acpiIsSdtUsable((const struct ts_acpiSdtHeader *)p_rsdt)) {
        debugPrint("acpi: RSDT checksum error.\n");
        return;
    }

    debugPrint("acpi: Exploring RSDT at ");
    debugPrintPointer(p_rsdt);
    debugPrint(".\n");

    const size_t l_rsdtHeaderLength = sizeof(struct ts_acpiRsdt);
    const int l_rsdtTableLength = (p_rsdt->a_header.a_length - l_rsdtHeaderLength) / 4;

    for(int l_index = 0; l_index < l_rsdtTableLength; l_index++) {
        acpiExploreTable((const struct ts_acpiSdtHeader *)(uintptr_t)p_rsdt->a_otherTables[l_index]);
    }
}

static void acpiExploreXsdt(const struct ts_acpiXsdt *p_xsdt) {
    if(!acpiIsSdtUsable((const struct ts_acpiSdtHeader *)p_xsdt)) {
        debugPrint("acpi: XSDT checksum error.\n");
        return;
    }

    debugPrint("acpi: Exploring XSDT at ");
    debugPrintPointer(p_xsdt);
    debugPrint(".\n");

    const size_t l_xsdtHeaderLength = sizeof(struct ts_acpiXsdt);
    const int l_xsdtTableLength = (p_xsdt->a_header.a_length - l_xsdtHeaderLength) / 8;

    for(int l_index = 0; l_index < l_xsdtTableLength; l_index++) {
        acpiExploreTable((const struct ts_acpiSdtHeader *)p_xsdt->a_otherTables[l_index]);
    }
}

static void acpiExploreTable(const struct ts_acpiSdtHeader *p_sdt) {
    debugPrint("acpi: Found table: \"");
    debugWrite(p_sdt->a_signature, 4);
    debugPrint("\" at ");
    debugPrintPointer(p_sdt);
    debugPrint(".\n");

    if(!acpiIsSdtUsable(p_sdt)) {
        debugPrint("acpi: Table checksum error.\n");
        return;
    }

    if(memcmp(p_sdt->a_signature, "FACP", 4) == 0) {
        s_acpiFadt = (const struct ts_acpiFadt *)p_sdt;

        uintptr_t l_a1 = (uintptr_t)s_acpiFadt;
        uintptr_t l_a2 = (uintptr_t)&s_acpiFadt->a_bootArchitectureFlags;
        uintptr_t l_a3 = l_a2 - l_a1;

        debugPrint("acpi: FADT boot arch flags offset: 0x");
        debugPrintHex64(l_a3);
        debugPrint("\n");

        debugPrint("acpi: FADT boot arch flags: 0x");
        debugPrintHex16(s_acpiFadt->a_bootArchitectureFlags);
        debugPrint("\n");
    }
}
