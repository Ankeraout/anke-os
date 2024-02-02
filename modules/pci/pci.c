#include <stdint.h>

#include "kernel/module.h"
#include "klibc/debug.h"

#include "kernel/arch/x86_64/inline.h"

#define C_PCI_IOPORT_ADDRESS 0xcf8
#define C_PCI_IOPORT_DATA 0xcfc

static void pciScan(void);
static void pciScanBus(uint8_t p_bus);
static void pciScanDevice(uint8_t p_bus, uint8_t p_slot);
static void pciScanFunction(uint8_t p_bus, uint8_t p_slot, uint8_t p_function);
static void pciSetAddress(uint32_t p_address);
static uint8_t pciRead8(uint32_t p_address);
static uint16_t pciRead16(uint32_t p_address);
static uint32_t pciRead32(uint32_t p_address);
static void pciWrite8(uint32_t p_address, uint8_t p_value);
static void pciWrite16(uint32_t p_address, uint16_t p_value);
static void pciWrite32(uint32_t p_address, uint32_t p_value);
static uint32_t pciGetAddress(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function
);

int init(void) {
    pciScan();
    return 0;
}

void exit(void) {
    kernelDebug("pci: Exiting!\n");
}

static void pciScan(void) {
    for(int l_bus = 0; l_bus < 256; l_bus++) {
        pciScanBus(l_bus);
    }
}

static void pciScanBus(uint8_t p_bus) {
    for(int l_slot = 0; l_slot < 32; l_slot++) {
        pciScanDevice(p_bus, l_slot);
    }
}

static void pciScanDevice(uint8_t p_bus, uint8_t p_slot) {
    uint32_t l_deviceAddress = pciGetAddress(p_bus, p_slot, 0);
    uint32_t l_deviceIdentifier = pciRead32(l_deviceAddress);

    if(l_deviceIdentifier == 0xffffffff) {
        // No device present
        return;
    }

    uint8_t l_headerType = pciRead8(l_deviceAddress + 13);

    if((l_headerType & (1 << 7)) != 0) {
        for(int l_function = 0; l_function < 8; l_function++) {
            pciScanFunction(p_bus, p_slot, l_function);
        }
    } else {
        pciScanFunction(p_bus, p_slot, 0);
    }
}

static void pciScanFunction(uint8_t p_bus, uint8_t p_slot, uint8_t p_function) {
    uint32_t l_deviceAddress = pciGetAddress(p_bus, p_slot, p_function);
    uint32_t l_deviceIdentifier = pciRead32(l_deviceAddress);

    if(l_deviceIdentifier == 0xffffffff) {
        // No device present
        return;
    }

    uint16_t l_deviceId = l_deviceIdentifier >> 16;
    uint16_t l_vendorId = l_deviceIdentifier;

    kernelDebug(
        "pci: %02x:%02x.%x: %04x:%04x\n",
        p_bus,
        p_slot,
        p_function,
        l_vendorId,
        l_deviceId
    );
}

static void pciSetAddress(uint32_t p_address) {
    outl(C_PCI_IOPORT_ADDRESS, (p_address & 0x00fffffc) | (1 << 31));
}

static uint8_t pciRead8(uint32_t p_address) {
    uint32_t l_shift = (3 - (p_address & 0x03)) << 3;
    pciSetAddress(p_address);
    uint32_t l_value = inl(C_PCI_IOPORT_DATA);

    return l_value >> l_shift;
}

static uint16_t pciRead16(uint32_t p_address) {
    uint32_t l_shift = (2 - (p_address & 0x02)) << 3;
    pciSetAddress(p_address);
    uint32_t l_value = inl(C_PCI_IOPORT_DATA);

    return l_value >> l_shift;
}

static uint32_t pciRead32(uint32_t p_address) {
    pciSetAddress(p_address);
    return inl(C_PCI_IOPORT_DATA);
}

static void pciWrite8(uint32_t p_address, uint8_t p_value) {
    uint32_t l_shift = (p_address & 0x03) << 3;
    uint32_t l_mask = ~(0xff << l_shift);

    pciSetAddress(p_address);

    uint32_t l_value = inl(C_PCI_IOPORT_DATA);
    l_value &= l_mask;
    l_value |= p_value << l_shift;

    outl(C_PCI_IOPORT_DATA, l_value);
}

static void pciWrite16(uint32_t p_address, uint16_t p_value) {
    uint32_t l_shift = (p_address & 0x02) << 3;
    uint32_t l_mask = ~(0xffff << l_shift);

    pciSetAddress(p_address);

    uint32_t l_value = inl(C_PCI_IOPORT_DATA);
    l_value &= l_mask;
    l_value |= p_value << l_shift;

    outl(C_PCI_IOPORT_DATA, l_value);
}

static void pciWrite32(uint32_t p_address, uint32_t p_value) {
    pciSetAddress(p_address);
    outl(C_PCI_IOPORT_DATA, p_value);
}

static uint32_t pciGetAddress(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function
) {
    return (p_bus << 16)
        | (p_slot << 11)
        | (p_function << 8);
}

M_DECLARE_MODULE("pci", init, exit);
