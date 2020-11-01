#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/i686/io.h"
#include "driver/pci.h"
#include "libk/libk.h"

uint16_t pci_readConfig16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    address = (lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | 0x80000000;

    outl(0xcf8, address);

    return (uint16_t)((inl(0xcfc) >> ((offset & 2) * 8)) & 0xffff);
}

int pci_init() {
    return 0;
}
