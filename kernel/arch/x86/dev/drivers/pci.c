#include <stdint.h>

#include <kernel/arch/x86/inline.h>
#include <kernel/dev/device.h>
#include <kernel/dev/pci.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/misc/list.h>
#include <kernel/debug.h>

#define C_IOPORT_PCI_CONFIG_ADDRESS 0xcf8
#define C_IOPORT_PCI_CONFIG_DATA 0xcfc

static int pciInit(struct ts_device *p_device);
static void pciInitDevice(
    struct ts_device *p_device,
    const struct ts_deviceAddressPci *p_deviceAddress
);
static void pciCheckDevice(
    struct ts_device *p_device,
    const struct ts_deviceAddressPci *p_deviceAddress
);
static void pciCheckDeviceFunction(
    struct ts_device *p_device,
    const struct ts_deviceAddressPci *p_deviceAddress
);
static uint32_t pciConfigGetAddress(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset
);
static uint8_t pciConfigRead8(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset
);
static uint16_t pciConfigRead16(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset
);
static uint32_t pciConfigRead32(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset
);
static void pciConfigWrite8(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset,
    uint8_t p_value
);
static void pciConfigWrite16(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset,
    uint16_t p_value
);
static void pciConfigWrite32(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset,
    uint32_t p_value
);
static size_t pciDriverApiGetChildCount(struct ts_device *p_device);
static struct ts_device *pciDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
);

struct ts_pciData {
    struct ts_arrayList a_children;
};

const struct ts_deviceDriverPci g_deviceDriverPci = {
    .a_base = {
        .a_name = "PCI bus controller",
        .a_api = {
            .a_init = pciInit,
            .a_getChild = pciDriverApiGetChild,
            .a_getChildCount = pciDriverApiGetChildCount,
            .a_isSupported = NULL
        }
    },
    .a_api = {
        .a_configRead8 = pciConfigRead8,
        .a_configRead16 = pciConfigRead16,
        .a_configRead32 = pciConfigRead32,
        .a_configWrite8 = pciConfigWrite8,
        .a_configWrite16 = pciConfigWrite16,
        .a_configWrite32 = pciConfigWrite32
    }
};

static int pciInit(struct ts_device *p_device) {
    p_device->a_driverData = kmalloc(sizeof(struct ts_pciData));

    if(p_device->a_driverData == NULL) {
        debug("pci: Failed to allocate memory for driver data.\n");
        return 1;
    }

    struct ts_pciData *l_data = (struct ts_pciData *)p_device->a_driverData;

    if(arrayListInit(&l_data->a_children) != 0) {
        debug("acpi: Failed to allocate memory for children list.\n");
        return 1;
    }

    for(int l_bus = 0; l_bus < 256; l_bus++) {
        for(int l_slot = 0; l_slot < 32; l_slot++) {
            struct ts_deviceAddressPci l_deviceAddress = {
                .a_common = {
                    .a_bus = E_DEVICEBUS_PCI
                },
                .a_bus = l_bus,
                .a_slot = l_slot,
                .a_func = 0
            };

            pciCheckDevice(p_device, &l_deviceAddress);
        }
    }

    debug("pci: PCI bus initialized.\n");

    return 0;
}

static void pciInitDevice(
    struct ts_device *p_device,
    const struct ts_deviceAddressPci *p_deviceAddress
) {
    struct ts_pciData *l_data = (struct ts_pciData *)p_device->a_driverData;

    uint16_t l_deviceId = pciConfigRead16(p_deviceAddress, 0);
    uint16_t l_vendorId = pciConfigRead16(p_deviceAddress, 2);
    uint8_t l_deviceClass = pciConfigRead8(p_deviceAddress, 8);
    uint8_t l_deviceSubclass = pciConfigRead8(p_deviceAddress, 9);
    uint8_t l_deviceProgrammingInterface = pciConfigRead8(p_deviceAddress, 10);
    uint8_t l_deviceRevision = pciConfigRead8(p_deviceAddress, 11);

    struct ts_device *l_device = kmalloc(sizeof(struct ts_device));

    if(l_device == NULL) {
        debug("pci: Failed to allocate memory for device.\n");
        return;
    }

    l_device->a_address = *(const struct ts_deviceAddress *)p_deviceAddress;
    l_device->a_parent = p_device;

    struct ts_deviceIdentifierPci l_identifier = {
        .a_base = {
            .a_bus = E_DEVICEBUS_PCI
        },
        .a_vendor = l_vendorId,
        .a_device = l_deviceId,
        .a_class = l_deviceClass,
        .a_subclass = l_deviceSubclass,
        .a_programmingInterface = l_deviceProgrammingInterface,
        .a_revision = l_deviceRevision
    };

    l_device->a_driver = deviceGetDriver(
        (struct ts_deviceIdentifier *)&l_identifier
    );

    debug(
        "pci: %02x:%02x.%x: %04x:%04x [%s]\n",
        p_deviceAddress->a_bus,
        p_deviceAddress->a_slot,
        p_deviceAddress->a_func,
        l_vendorId,
        l_deviceId,
        l_device->a_driver->a_name
    );

    if(l_device->a_driver->a_api.a_init(l_device) == 0) {
        arrayListAdd(&l_data->a_children, l_device);
    }
}

static void pciCheckDevice(
    struct ts_device *p_device,
    const struct ts_deviceAddressPci *p_deviceAddress
) {
    uint16_t l_deviceId = pciConfigRead16(p_deviceAddress, 0);
    uint16_t l_vendorId = pciConfigRead16(p_deviceAddress, 2);

    if((l_deviceId == 0xffff) && (l_vendorId == 0xffff)) {
        // No device present
        return;
    }

    uint8_t l_headerType = pciConfigRead8(p_deviceAddress, 13);

    if((l_headerType & 0x80) != 0) {
        for(int l_func = 0; l_func < 8; l_func++) {
            struct ts_deviceAddressPci l_deviceAddress = *p_deviceAddress;

            l_deviceAddress.a_func = l_func;

            pciCheckDeviceFunction(p_device, &l_deviceAddress);
        }
    } else {
        pciCheckDeviceFunction(p_device, p_deviceAddress);
    }
}

static void pciCheckDeviceFunction(
    struct ts_device *p_device,
    const struct ts_deviceAddressPci *p_deviceAddress
) {
    uint16_t l_deviceId = pciConfigRead16(p_deviceAddress, 0);
    uint16_t l_vendorId = pciConfigRead16(p_deviceAddress, 2);

    if((l_deviceId == 0xffff) && (l_vendorId == 0xffff)) {
        // No device present
        return;
    }

    pciInitDevice(p_device, p_deviceAddress);
}

static uint32_t pciConfigGetAddress(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset
) {
    return (p_deviceAddress->a_bus << 16)
        | (p_deviceAddress->a_slot << 11)
        | (p_deviceAddress->a_func << 8)
        | (p_offset & 0xfc)
        | 0x80000000;
}

static uint8_t pciConfigRead8(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset
) {
    const uint32_t l_address = pciConfigGetAddress(p_deviceAddress, p_offset);
    const int l_shift = (3 - (p_offset & 0x03)) << 3;

    outl(C_IOPORT_PCI_CONFIG_ADDRESS, l_address);

    return (inl(C_IOPORT_PCI_CONFIG_DATA) >> l_shift) & 0xff;
}

static uint16_t pciConfigRead16(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset
) {
    const uint32_t l_address = pciConfigGetAddress(p_deviceAddress, p_offset);
    const int l_shift = (2 - (p_offset & 0x02)) << 3;

    outl(C_IOPORT_PCI_CONFIG_ADDRESS, l_address);

    return (inl(C_IOPORT_PCI_CONFIG_DATA) >> l_shift) & 0xffff;
}

static uint32_t pciConfigRead32(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset
) {
    const uint32_t l_address = pciConfigGetAddress(p_deviceAddress, p_offset);

    outl(C_IOPORT_PCI_CONFIG_ADDRESS, l_address);

    return inl(C_IOPORT_PCI_CONFIG_DATA);
}

static void pciConfigWrite8(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset,
    uint8_t p_value
) {
    const uint32_t l_address = pciConfigGetAddress(p_deviceAddress, p_offset);
    const int l_shift = (p_offset & 0x03) << 3;
    const uint32_t l_mask = ~(0xff << l_shift);

    outl(C_IOPORT_PCI_CONFIG_ADDRESS, l_address);

    uint32_t l_tmp = inl(C_IOPORT_PCI_CONFIG_DATA);
    l_tmp &= l_mask;
    l_tmp |= p_value << l_shift;

    outl(C_IOPORT_PCI_CONFIG_DATA, l_tmp);
}

static void pciConfigWrite16(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset,
    uint16_t p_value
) {
    const uint32_t l_address = pciConfigGetAddress(p_deviceAddress, p_offset);
    const int l_shift = (p_offset & 0x02) << 3;
    const uint32_t l_mask = ~(0xffff << l_shift);

    outl(C_IOPORT_PCI_CONFIG_ADDRESS, l_address);

    uint32_t l_tmp = inl(C_IOPORT_PCI_CONFIG_DATA);
    l_tmp &= l_mask;
    l_tmp |= p_value << l_shift;

    outl(C_IOPORT_PCI_CONFIG_DATA, l_tmp);
}

static void pciConfigWrite32(
    const struct ts_deviceAddressPci *p_deviceAddress,
    uint8_t p_offset,
    uint32_t p_value
) {
    const uint32_t l_address = pciConfigGetAddress(p_deviceAddress, p_offset);

    outl(C_IOPORT_PCI_CONFIG_ADDRESS, l_address);
    outl(C_IOPORT_PCI_CONFIG_DATA, p_value);
}

static size_t pciDriverApiGetChildCount(struct ts_device *p_device) {
    struct ts_pciData *l_data = (struct ts_pciData *)p_device->a_driverData;

    return arrayListGetLength(&l_data->a_children);
}

static struct ts_device *pciDriverApiGetChild(
    struct ts_device *p_device,
    size_t p_index
) {
    struct ts_pciData *l_data = (struct ts_pciData *)p_device->a_driverData;

    return arrayListGet(&l_data->a_children, p_index);
}
