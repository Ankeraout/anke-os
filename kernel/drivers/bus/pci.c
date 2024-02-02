#include <stdint.h>

#include "kernel/arch/x86_64/inline.h"
#include "kernel/device.h"
#include "kernel/drivers/bus/pci.h"
#include "klibc/debug.h"

#define C_PCI_IOPORT_ADDRESS 0xcf8
#define C_PCI_IOPORT_DATA 0xcfc

static void pciSetAddress(uint32_t p_address);
static void pciScanBus(
    uint8_t p_bus,
    void (*p_callback)(
        uint8_t p_bus,
        uint8_t p_slot,
        uint8_t p_function,
        uint16_t p_vendor,
        uint16_t p_device,
        uint8_t p_class,
        uint8_t p_subclass,
        uint8_t p_programmingInterface
    )
);
static void pciScanSlot(
    uint8_t p_bus,
    uint8_t p_slot,
    void (*p_callback)(
        uint8_t p_bus,
        uint8_t p_slot,
        uint8_t p_function,
        uint16_t p_vendor,
        uint16_t p_device,
        uint8_t p_class,
        uint8_t p_subclass,
        uint8_t p_programmingInterface
    )
);
static void pciScanFunction(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    void (*p_callback)(
        uint8_t p_bus,
        uint8_t p_slot,
        uint8_t p_function,
        uint16_t p_vendor,
        uint16_t p_device,
        uint8_t p_class,
        uint8_t p_subclass,
        uint8_t p_programmingInterface
    )
);
static void pciInitCallback(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint16_t p_vendor,
    uint16_t p_device,
    uint8_t p_class,
    uint8_t p_subclass,
    uint8_t p_programmingInterface
);

int pciInit(void) {
    kernelDebug("pci: PCI devices:\n");
    kernelDebug("pci: +---------+--------+--------+-------+----------+--------+\n");
    kernelDebug("pci: | Address | Vendor | Device | Class | Subclass | ProgIF |\n");
    kernelDebug("pci: +---------+--------+--------+-------+----------+--------+\n");
    pciScan(pciInitCallback);
    kernelDebug("pci: +---------+--------+--------+-------+----------+--------+\n");

    return 0;
}

void pciScan(
    void (*p_callback)(
        uint8_t p_bus,
        uint8_t p_slot,
        uint8_t p_function,
        uint16_t p_vendor,
        uint16_t p_device,
        uint8_t p_class,
        uint8_t p_subclass,
        uint8_t p_programmingInterface
    )
) {
    for(int l_bus = 0; l_bus < 256; l_bus++) {
        pciScanBus(l_bus, p_callback);
    }
}

uint8_t pciRead8(uint32_t p_address) {
    uint32_t l_shift = (3 - (p_address & 0x03)) << 3;
    pciSetAddress(p_address);
    uint32_t l_value = inl(C_PCI_IOPORT_DATA);

    return l_value >> l_shift;
}

uint16_t pciRead16(uint32_t p_address) {
    uint32_t l_shift = (2 - (p_address & 0x02)) << 3;
    pciSetAddress(p_address);
    uint32_t l_value = inl(C_PCI_IOPORT_DATA);

    return l_value >> l_shift;
}

uint32_t pciRead32(uint32_t p_address) {
    pciSetAddress(p_address);
    return inl(C_PCI_IOPORT_DATA);
}

void pciWrite8(uint32_t p_address, uint8_t p_value) {
    uint32_t l_shift = (p_address & 0x03) << 3;
    uint32_t l_mask = ~(0xff << l_shift);

    pciSetAddress(p_address);

    uint32_t l_value = inl(C_PCI_IOPORT_DATA);
    l_value &= l_mask;
    l_value |= p_value << l_shift;

    outl(C_PCI_IOPORT_DATA, l_value);
}

void pciWrite16(uint32_t p_address, uint16_t p_value) {
    uint32_t l_shift = (p_address & 0x02) << 3;
    uint32_t l_mask = ~(0xffff << l_shift);

    pciSetAddress(p_address);

    uint32_t l_value = inl(C_PCI_IOPORT_DATA);
    l_value &= l_mask;
    l_value |= p_value << l_shift;

    outl(C_PCI_IOPORT_DATA, l_value);
}

void pciWrite32(uint32_t p_address, uint32_t p_value) {
    pciSetAddress(p_address);
    outl(C_PCI_IOPORT_DATA, p_value);
}

uint32_t pciGetAddress(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function
) {
    return (p_bus << 16)
        | (p_slot << 11)
        | (p_function << 8);
}

static void pciSetAddress(uint32_t p_address) {
    outl(C_PCI_IOPORT_ADDRESS, (p_address & 0x00fffffc) | (1 << 31));
}

static void pciScanBus(
    uint8_t p_bus,
    void (*p_callback)(
        uint8_t p_bus,
        uint8_t p_slot,
        uint8_t p_function,
        uint16_t p_vendor,
        uint16_t p_device,
        uint8_t p_class,
        uint8_t p_subclass,
        uint8_t p_programmingInterface
    )
) {
    for(uint8_t l_slot = 0; l_slot < 32; l_slot++) {
        pciScanSlot(p_bus, l_slot, p_callback);
    }
}

static void pciScanSlot(
    uint8_t p_bus,
    uint8_t p_slot,
    void (*p_callback)(
        uint8_t p_bus,
        uint8_t p_slot,
        uint8_t p_function,
        uint16_t p_vendor,
        uint16_t p_device,
        uint8_t p_class,
        uint8_t p_subclass,
        uint8_t p_programmingInterface
    )
) {
    uint32_t l_deviceAddress = pciGetAddress(p_bus, p_slot, 0);
    uint32_t l_deviceIdentifier = pciRead32(l_deviceAddress);

    if(l_deviceIdentifier == 0xffffffff) {
        // No device present
        return;
    }

    uint8_t l_headerType = pciRead8(l_deviceAddress + 13);
    uint8_t l_functionCount;

    if((l_headerType & (1 << 7)) != 0) {
        l_functionCount = 8;
    } else {
        l_functionCount = 1;
    }

    for(uint8_t l_function = 0; l_function < l_functionCount; l_function++) {
        pciScanFunction(p_bus, p_slot, l_function, p_callback);
    }
}

static void pciScanFunction(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    void (*p_callback)(
        uint8_t p_bus,
        uint8_t p_slot,
        uint8_t p_function,
        uint16_t p_vendor,
        uint16_t p_device,
        uint8_t p_class,
        uint8_t p_subclass,
        uint8_t p_programmingInterface
    )
) {
    uint32_t l_deviceAddress = pciGetAddress(p_bus, p_slot, p_function);
    uint32_t l_deviceIdentifier = pciRead32(l_deviceAddress);

    if(l_deviceIdentifier == 0xffffffff) {
        // No device present
        return;
    }

    uint16_t l_deviceId = l_deviceIdentifier >> 16;
    uint16_t l_vendorId = l_deviceIdentifier;

    uint32_t l_deviceCodes = pciRead32(l_deviceAddress + 8);

    uint8_t l_class = l_deviceCodes >> 24;
    uint8_t l_subclass = l_deviceCodes >> 16;
    uint8_t l_programmingInterface = l_deviceCodes >> 8;

    p_callback(
        p_bus,
        p_slot,
        p_function,
        l_vendorId,
        l_deviceId,
        l_class,
        l_subclass,
        l_programmingInterface
    );
}

static void pciInitCallback(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint16_t p_vendor,
    uint16_t p_device,
    uint8_t p_class,
    uint8_t p_subclass,
    uint8_t p_programmingInterface
) {
    kernelDebug(
        "pci: | %02x:%02x:%x | 0x%04x | 0x%04x | 0x%02x  | 0x%02x     | 0x%02x   |\n",
        p_bus,
        p_slot,
        p_function,
        p_vendor,
        p_device,
        p_class,
        p_subclass,
        p_programmingInterface
    );
}
