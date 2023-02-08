#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/x86/inline.h"
#include "debug.h"

#define C_IOPORT_PCI_CONFIG_ADDRESS 0xcf8
#define C_IOPORT_PCI_CONFIG_DATA 0xcfc

static void pciEnumerateDevice(uint8_t p_bus, uint8_t p_slot, uint8_t p_func);
static void pciCheckDevice(uint8_t p_bus, uint8_t p_slot);
static void pciCheckDeviceFunction(uint8_t p_bus, uint8_t p_slot, uint8_t p_func);
static uint32_t pciConfigGetAddress(uint8_t p_bus, uint8_t p_slot, uint8_t p_func, uint8_t p_offset);
static uint8_t pciConfigReadByte(uint8_t p_bus, uint8_t p_slot, uint8_t p_func, uint8_t p_offset);
static uint16_t pciConfigReadWord(uint8_t p_bus, uint8_t p_slot, uint8_t p_func, uint8_t p_offset);
static uint32_t pciConfigReadDword(uint8_t p_bus, uint8_t p_slot, uint8_t p_func, uint8_t p_offset);
static void pciConfigWriteByte(uint8_t p_bus, uint8_t p_slot, uint8_t p_func, uint8_t p_offset, uint8_t p_value);
static void pciConfigWriteWord(uint8_t p_bus, uint8_t p_slot, uint8_t p_func, uint8_t p_offset, uint16_t p_value);
static void pciConfigWriteDword(uint8_t p_bus, uint8_t p_slot, uint8_t p_func, uint8_t p_offset, uint32_t p_value);

int pciInit(void) {
    for(int l_bus = 0; l_bus < 256; l_bus++) {
        for(int l_slot = 0; l_slot < 32; l_slot++) {
            pciCheckDevice(l_bus, l_slot);
        }
    }

    return 0;
}

static void pciEnumerateDevice(uint8_t p_bus, uint8_t p_slot, uint8_t p_func) {
    uint16_t l_deviceId = pciConfigReadWord(p_bus, p_slot, p_func, 0);
    uint16_t l_vendorId = pciConfigReadWord(p_bus, p_slot, p_func, 2);
    uint8_t l_deviceClass = pciConfigReadByte(p_bus, p_slot, p_func, 8);
    uint8_t l_deviceSubclass = pciConfigReadByte(p_bus, p_slot, p_func, 9);
    uint8_t l_devicePif = pciConfigReadByte(p_bus, p_slot, p_func, 10);
    uint8_t l_deviceRevision = pciConfigReadByte(p_bus, p_slot, p_func, 11);

    debugPrint("pci: ");
    debugPrintHex8(p_bus);
    debugPrint(":");
    debugPrintHex8(p_slot);
    debugPrint(".");
    debugWrite(&"01234567"[p_func], 1);
    debugPrint(": ");
    debugPrintHex16(l_vendorId);
    debugPrint(":");
    debugPrintHex16(l_deviceId);
    debugPrint("\n");
}

static void pciCheckDevice(uint8_t p_bus, uint8_t p_slot) {
    uint16_t l_deviceId = pciConfigReadWord(p_bus, p_slot, 0, 0);
    uint16_t l_vendorId = pciConfigReadWord(p_bus, p_slot, 0, 2);

    if((l_deviceId == 0xffff) && (l_vendorId == 0xffff)) {
        // No device present
        return;
    }

    uint8_t l_headerType = pciConfigReadByte(p_bus, p_slot, 0, 13);

    if((l_headerType & 0x80) != 0) {
        for(int l_func = 0; l_func < 8; l_func++) {
            pciCheckDeviceFunction(p_bus, p_slot, l_func);
        }
    } else {
        pciCheckDeviceFunction(p_bus, p_slot, 0);
    }
}

static void pciCheckDeviceFunction(uint8_t p_bus, uint8_t p_slot, uint8_t p_func) {
    uint16_t l_deviceId = pciConfigReadWord(p_bus, p_slot, p_func, 0);
    uint16_t l_vendorId = pciConfigReadWord(p_bus, p_slot, p_func, 2);

    if((l_deviceId == 0xffff) && (l_vendorId == 0xffff)) {
        // No device present
        return;
    }

    pciEnumerateDevice(p_bus, p_slot, p_func);
}

static uint32_t pciConfigGetAddress(uint8_t p_bus, uint8_t p_slot, uint8_t p_func, uint8_t p_offset) {
    return (p_bus << 16)
        | (p_slot << 11)
        | (p_func << 8)
        | (p_offset & 0xfc)
        | 0x80000000;
}

static uint8_t pciConfigReadByte(uint8_t p_bus, uint8_t p_slot, uint8_t p_func, uint8_t p_offset) {
    const uint32_t l_address = pciConfigGetAddress(p_bus, p_slot, p_func, p_offset);
    const int l_shift = (3 - (p_offset & 0x03)) << 3;

    outl(C_IOPORT_PCI_CONFIG_ADDRESS, l_address);

    return (inl(C_IOPORT_PCI_CONFIG_DATA) >> l_shift) & 0xff;
}

static uint16_t pciConfigReadWord(uint8_t p_bus, uint8_t p_slot, uint8_t p_func, uint8_t p_offset) {
    const uint32_t l_address = pciConfigGetAddress(p_bus, p_slot, p_func, p_offset);
    const int l_shift = (2 - (p_offset & 0x02)) << 3;

    outl(C_IOPORT_PCI_CONFIG_ADDRESS, l_address);

    return (inl(C_IOPORT_PCI_CONFIG_DATA) >> l_shift) & 0xffff;
}

static uint32_t pciConfigReadDword(uint8_t p_bus, uint8_t p_slot, uint8_t p_func, uint8_t p_offset) {
    const uint32_t l_address = pciConfigGetAddress(p_bus, p_slot, p_func, p_offset);

    outl(C_IOPORT_PCI_CONFIG_ADDRESS, l_address);

    return inl(C_IOPORT_PCI_CONFIG_DATA);
}

static void pciConfigWriteByte(uint8_t p_bus, uint8_t p_slot, uint8_t p_func, uint8_t p_offset, uint8_t p_value) {
    const uint32_t l_address = pciConfigGetAddress(p_bus, p_slot, p_func, p_offset);
    const int l_shift = (p_offset & 0x03) << 3;
    const uint32_t l_mask = ~(0xff << l_shift);

    outl(C_IOPORT_PCI_CONFIG_ADDRESS, l_address);

    uint32_t l_tmp = inl(C_IOPORT_PCI_CONFIG_DATA);
    l_tmp &= l_mask;
    l_tmp |= p_value << l_shift;

    outl(C_IOPORT_PCI_CONFIG_DATA, l_tmp);
}

static void pciConfigWriteWord(uint8_t p_bus, uint8_t p_slot, uint8_t p_func, uint8_t p_offset, uint16_t p_value) {
    const uint32_t l_address = pciConfigGetAddress(p_bus, p_slot, p_func, p_offset);
    const int l_shift = (p_offset & 0x02) << 3;
    const uint32_t l_mask = ~(0xffff << l_shift);

    outl(C_IOPORT_PCI_CONFIG_ADDRESS, l_address);

    uint32_t l_tmp = inl(C_IOPORT_PCI_CONFIG_DATA);
    l_tmp &= l_mask;
    l_tmp |= p_value << l_shift;

    outl(C_IOPORT_PCI_CONFIG_DATA, l_tmp);
}

static void pciConfigWriteDword(uint8_t p_bus, uint8_t p_slot, uint8_t p_func, uint8_t p_offset, uint32_t p_value) {
    const uint32_t l_address = pciConfigGetAddress(p_bus, p_slot, p_func, p_offset);

    outl(C_IOPORT_PCI_CONFIG_ADDRESS, l_address);
    outl(C_IOPORT_PCI_CONFIG_DATA, p_value);
}
