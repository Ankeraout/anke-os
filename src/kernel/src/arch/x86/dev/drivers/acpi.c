#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/arch.h"
#include "arch/x86/inline.h"
#include "arch/x86/isr.h"
#include "arch/x86/dev/drivers/acpi.h"
#include "arch/x86/dev/drivers/i8042.h"
#include "arch/x86/dev/drivers/i8254.h"
#include "arch/x86/dev/drivers/i8259.h"
#include "arch/x86/dev/drivers/pci.h"
#include "dev/device.h"
#include "dev/pci.h"
#include "dev/timer.h"
#include "klibc/list.h"
#include "klibc/stdlib.h"
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

static int acpiInit(struct ts_device *p_device);
static void acpiFindRsdp(struct ts_device *p_device);
static void acpiFindRsdpAt(
    struct ts_device *p_device,
    const void *p_ptr,
    size_t p_regionSize
);
static bool acpiIsSdtUsable(const struct ts_acpiSdtHeader *p_table);
static uint8_t acpiComputeChecksum(const void *p_table, size_t p_length);
static void acpiExploreRsdt(
    struct ts_device *p_device,
    const struct ts_acpiRsdt *p_rsdt
);
static void acpiExploreXsdt(
    struct ts_device *p_device,
    const struct ts_acpiXsdt *p_xsdt
);
static void acpiExploreTable(
    struct ts_device *p_device,
    const struct ts_acpiSdtHeader *p_sdt
);
static bool acpiIs8042Present(
    struct ts_device *p_device
);
static struct ts_device *acpiDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_childIndex
);
static size_t acpiDriverApiGetChildCount(struct ts_device *p_device);

static const char s_rsdpSignature[8] = "RSD PTR ";
static const size_t s_rsdpSignatureLength = 8;
const struct ts_deviceDriver g_deviceDriverAcpi = {
    .a_name = "ACPI root bus",
    .a_api = {
        .a_init = acpiInit,
        .a_getChildCount = acpiDriverApiGetChildCount,
        .a_getChild = acpiDriverApiGetChild,
        .a_isSupported = NULL
    }
};

struct ts_acpiDeviceDriverData {
    const struct ts_acpiRsdp2 *a_acpiRsdp;
    int a_acpiRsdpVersion;
    const struct ts_acpiFadt *a_acpiFadt;
    struct ts_list a_children;
};

static struct ts_acpiDeviceDriverData s_acpiDeviceDriverData;

static int acpiInit(struct ts_device *p_device) {
    p_device->a_driverData = &s_acpiDeviceDriverData;

    struct ts_acpiDeviceDriverData *l_data =
        (struct ts_acpiDeviceDriverData *)&p_device->a_driverData;

    if(listInit(&l_data->a_children) == NULL) {
        debugPrint("acpi: Failed to allocate memory for children list.\n");
        return 1;
    }

    acpiFindRsdp(p_device);

    if(l_data->a_acpiRsdp == NULL) {
        // No ACPI
        debugPrint("acpi: RSDP was not found.\n");
        return 1;
    }

    debugPrint("acpi: ACPI RSDP was found at 0x");
    debugPrintPointer(l_data->a_acpiRsdp);
    debugPrint(".\n");

    if(l_data->a_acpiRsdpVersion == 1) {
        debugPrint("acpi: ACPI RSDP 1.0 structure found.\n");
        acpiExploreRsdt(
            p_device,
            (const struct ts_acpiRsdt *)(uintptr_t)l_data->a_acpiRsdp->rsdp1.a_rsdtAddress
        );
    } else if(l_data->a_acpiRsdpVersion == 2) {
        debugPrint("acpi: ACPI RSDP 2.0 structure found.\n");
        acpiExploreXsdt(
            p_device,
            (const struct ts_acpiXsdt *)(uintptr_t)l_data->a_acpiRsdp->a_xsdtAddress
        );
    } else {
        debugPrint("acpi: Unknown ACPI RSDP version found.\n");
        return 1;
    }

    // Initialize PIC
    struct ts_device *l_pic = kmalloc(sizeof(struct ts_device));

    if(l_pic == NULL) {
        debugPrint("acpi: Failed to allocate memory for PIC device.\n");
        return 1;
    }

    l_pic->a_parent = p_device;
    l_pic->a_address.a_common.a_bus = E_DEVICEBUS_ROOT;
    l_pic->a_driver = (const struct ts_deviceDriver *)&g_deviceDriverI8259;
    l_pic->a_driver->a_api.a_init(l_pic);
    listAdd(&l_data->a_children, l_pic);

    isrInit(l_pic);

    // Enable interrupts
    archInterruptsEnable();

    // Initialize PIT
    struct ts_device *l_pit = kmalloc(sizeof(struct ts_device));

    if(l_pit == NULL) {
        debugPrint("acpi: Failed to allocate memory for PIT device.\n");
        return 1;
    }

    l_pit->a_driver = (const struct ts_deviceDriver *)&g_deviceDriverI8254;
    l_pit->a_address.a_common.a_bus = E_DEVICEBUS_ROOT;
    l_pit->a_parent = p_device;
    l_pit->a_driver->a_api.a_init(l_pit);
    listAdd(&l_data->a_children, l_pit);

    timerSetDevice(l_pit);

    // Initialize PCI controller
    struct ts_device *l_pciController = kmalloc(sizeof(struct ts_device));

    if(l_pciController == NULL) {
        debugPrint("acpi: Failed to allocate memory for PCI controller device.\n");
        return 1;
    }

    l_pciController->a_driver = &g_deviceDriverPci;
    l_pciController->a_address.a_common.a_bus = E_DEVICEBUS_ROOT;
    l_pciController->a_parent = p_device;
    l_pciController->a_driver->a_api.a_init(l_pciController);
    listAdd(&l_data->a_children, l_pciController);

    // Initialize PS/2 controller
    if(acpiIs8042Present(p_device)) {
        struct ts_device *l_ps2Controller = kmalloc(sizeof(struct ts_device));

        if(l_ps2Controller == NULL) {
            debugPrint("acpi: Failed to allocate memory for PS/2 controller device.\n");
            return 1;
        }

        l_ps2Controller->a_driver = &g_deviceDriverI8042;
        l_ps2Controller->a_address.a_common.a_bus = E_DEVICEBUS_ROOT;
        l_ps2Controller->a_parent = p_device;
        l_ps2Controller->a_driver->a_api.a_init(l_ps2Controller);
        listAdd(&l_data->a_children, l_ps2Controller);
    }

    return 0;
}

static void acpiFindRsdp(struct ts_device *p_device) {
    acpiFindRsdpAt(
        p_device,
        (const void *)0x00000000000e0000,
        0x20000
    );
}

static void acpiFindRsdpAt(
    struct ts_device *p_device,
    const void *p_ptr,
    size_t p_regionSize
) {
    struct ts_acpiDeviceDriverData *l_data =
        (struct ts_acpiDeviceDriverData *)&p_device->a_driverData;

    for(size_t l_index = 0; l_index < p_regionSize; l_index += 16) {
        const struct ts_acpiRsdp2 *l_castedPtr =
            (const struct ts_acpiRsdp2 *)&(((const uint8_t *)p_ptr)[l_index]);

        if(memcmp(l_castedPtr, s_rsdpSignature, s_rsdpSignatureLength) == 0) {
            // Compute checksum
            if(acpiComputeChecksum(l_castedPtr, sizeof(struct ts_acpiRsdp)) != 0) {
                continue;
            }

            // Valid ACPI RSDP 1.0 structure found.
            l_data->a_acpiRsdpVersion = 1;
            l_data->a_acpiRsdp = (const struct ts_acpiRsdp2 *)l_castedPtr;

            if(l_data->a_acpiRsdp->rsdp1.a_revision == 0) {
                // ACPI version is 1.0, we do not need to check for ACPI 2.0
                // structure fields.
                return;
            }

            // ACPI version is 2.0+, so we check for ACPI RSDP 2.0 structure
            // fields.
            if(acpiComputeChecksum(l_castedPtr, sizeof(struct ts_acpiRsdp2)) != 0) {
                l_data->a_acpiRsdpVersion = 1;
            } else {
                l_data->a_acpiRsdpVersion = 2;
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

static void acpiExploreRsdt(
    struct ts_device *p_device,
    const struct ts_acpiRsdt *p_rsdt
) {
    if(!acpiIsSdtUsable((const struct ts_acpiSdtHeader *)p_rsdt)) {
        debugPrint("acpi: RSDT checksum error.\n");
        return;
    }

    debugPrint("acpi: Exploring RSDT at 0x");
    debugPrintPointer(p_rsdt);
    debugPrint(".\n");

    const size_t l_rsdtHeaderLength = sizeof(struct ts_acpiRsdt);
    const int l_rsdtTableLength = (p_rsdt->a_header.a_length - l_rsdtHeaderLength) / 4;

    for(int l_index = 0; l_index < l_rsdtTableLength; l_index++) {
        acpiExploreTable(
            p_device,
            (const struct ts_acpiSdtHeader *)(uintptr_t)p_rsdt->a_otherTables[l_index]
        );
    }
}

static void acpiExploreXsdt(
    struct ts_device *p_device,
    const struct ts_acpiXsdt *p_xsdt
) {
    if(!acpiIsSdtUsable((const struct ts_acpiSdtHeader *)p_xsdt)) {
        debugPrint("acpi: XSDT checksum error.\n");
        return;
    }

    debugPrint("acpi: Exploring XSDT at 0x");
    debugPrintPointer(p_xsdt);
    debugPrint(".\n");

    const size_t l_xsdtHeaderLength = sizeof(struct ts_acpiXsdt);
    const int l_xsdtTableLength = (p_xsdt->a_header.a_length - l_xsdtHeaderLength) / 8;

    for(int l_index = 0; l_index < l_xsdtTableLength; l_index++) {
        acpiExploreTable(
            p_device,
            (const struct ts_acpiSdtHeader *)p_xsdt->a_otherTables[l_index]
        );
    }
}

static void acpiExploreTable(
    struct ts_device *p_device,
    const struct ts_acpiSdtHeader *p_sdt
) {
    struct ts_acpiDeviceDriverData *l_data =
        (struct ts_acpiDeviceDriverData *)&p_device->a_driverData;

    debugPrint("acpi: Found table: \"");
    debugWrite(p_sdt->a_signature, 4);
    debugPrint("\" at 0x");
    debugPrintPointer(p_sdt);
    debugPrint(".\n");

    if(!acpiIsSdtUsable(p_sdt)) {
        debugPrint("acpi: Table checksum error.\n");
        return;
    }

    if(memcmp(p_sdt->a_signature, "FACP", 4) == 0) {
        l_data->a_acpiFadt = (const struct ts_acpiFadt *)p_sdt;
    }
}

static bool acpiIs8042Present(
    struct ts_device *p_device
) {
    struct ts_acpiDeviceDriverData *l_data =
        (struct ts_acpiDeviceDriverData *)&p_device->a_driverData;

    if(l_data->a_acpiRsdp == NULL) {
        // ACPI is not supported => 8042 controller is present
        return true;
    }

    if(l_data->a_acpiFadt == NULL) {
        // FADT not found => 8042 controller is present
        return true;
    }

    if(l_data->a_acpiRsdp->rsdp1.a_revision == 0) {
        // ACPI 1.0 => boot architecture flags field is invalid.
        return true;
    }

    return (l_data->a_acpiFadt->a_bootArchitectureFlags & (1 << 1)) != 0;
}

static struct ts_device *acpiDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_childIndex
) {
    struct ts_acpiDeviceDriverData *l_data =
        (struct ts_acpiDeviceDriverData *)&p_device->a_driverData;

    return (struct ts_device *)listGet(&l_data->a_children, p_childIndex);
}

static size_t acpiDriverApiGetChildCount(struct ts_device *p_device) {
    struct ts_acpiDeviceDriverData *l_data =
        (struct ts_acpiDeviceDriverData *)&p_device->a_driverData;

    return listGetLength(&l_data->a_children);
}
