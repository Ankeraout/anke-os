#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/i686/io.h"
#include "debug.h"
#include "driver/pci.h"
#include "libk/libk.h"

#define PCI_CSAM2_CONFIG_ADDRESS 0xcf8
#define PCI_CSAM2_CONFIG_DATA 0xcfc
#define PCI_VENDOR_NO_DEVICE 0xffff
#define PCI_HEADERTYPE_FLAG_MULTIFUNCTION 0x80

uint32_t pci_csam2_readConfig32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    address = (lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | 0x80000000;

    outl(PCI_CSAM2_CONFIG_ADDRESS, address);

    return inl(PCI_CSAM2_CONFIG_DATA);
}

uint16_t pci_csam2_readConfig16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    return (uint16_t)((pci_csam2_readConfig32(bus, slot, func, offset) >> (16 - ((offset & 2) << 3))) & 0xffff);
}

uint8_t pci_csam2_readConfig8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    return (uint8_t)((pci_csam2_readConfig32(bus, slot, func, offset) >> (24 - ((offset & 3) << 3))) & 0xff);
}

uint8_t pci_csam2_getHeaderType(uint8_t bus, uint8_t slot, uint8_t function) {
    return pci_csam2_readConfig8(bus, slot, function, 0x0d);
}

void pci_csam2_printDevice(uint8_t bus, uint8_t slot, uint8_t function) {
    char buffer[5];

    kernel_debug("PCI device found at ");
    kernel_debug(hex8(bus, buffer));
    kernel_debug(":");
    kernel_debug(hex8(slot, buffer));
    kernel_debug(".");
    kernel_debug(itoa(function, buffer, 10));
    kernel_debug(" ven=0x");
    kernel_debug(hex16(pci_csam2_readConfig16(bus, slot, function, 2), buffer));
    kernel_debug(" dev=0x");
    kernel_debug(hex16(pci_csam2_readConfig16(bus, slot, function, 0), buffer));
    kernel_debug(" class=0x");
    kernel_debug(hex8(pci_csam2_readConfig8(bus, slot, function, 8), buffer));
    kernel_debug(" sub=0x");
    kernel_debug(hex8(pci_csam2_readConfig8(bus, slot, function, 9), buffer));
    kernel_debug(" pif=0x");
    kernel_debug(hex8(pci_csam2_readConfig8(bus, slot, function, 10), buffer));
    kernel_debug("\n");
}

void pci_csam2_enumerate() {
    for(uint16_t bus = 0; bus < 256; bus++) {
        for(uint8_t device = 0; device < 32; device++) {
            uint16_t vendor = pci_csam2_readConfig16(bus, device, 0, 2);

            if(vendor == PCI_VENDOR_NO_DEVICE) {
                continue;
            }

            uint8_t headerType = pci_csam2_readConfig8(bus, device, 0, 0x0d);

            pci_csam2_printDevice(bus, device, 0);

            if(headerType & PCI_HEADERTYPE_FLAG_MULTIFUNCTION) {
                for(int function = 1; function < 8; function++) {
                    vendor = pci_csam2_readConfig16(bus, device, function, 2);

                    if(vendor == PCI_VENDOR_NO_DEVICE) {
                        continue;
                    }

                    pci_csam2_printDevice(bus, device, function);
                }
            }
        }
    }
}

int pci_init(pci_csam_t csam) {
    if(csam == PCI_CSAM_2) {
        pci_csam2_enumerate();
    }

    return 0;
}
