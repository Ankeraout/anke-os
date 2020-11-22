#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/i686/io.hpp"
#include "irq.hpp"
#include "debug.hpp"
#include "driver/pci.hpp"
#include "driver/rtl8139.hpp"
#include "libk/libk.hpp"

#define PCI_CSAM2_CONFIG_ADDRESS 0xcf8
#define PCI_CSAM2_CONFIG_DATA 0xcfc
#define PCI_VENDOR_NO_DEVICE 0xffff
#define PCI_HEADERTYPE_FLAG_MULTIFUNCTION 0x80

namespace kernel {
    uint32_t pci_csam2_readConfig32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
        uint32_t address;
        uint32_t lbus = (uint32_t)bus;
        uint32_t lslot = (uint32_t)slot;
        uint32_t lfunc = (uint32_t)func;

        address = (lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | 0x80000000;

        outl(PCI_CSAM2_CONFIG_ADDRESS, address);

        return inl(PCI_CSAM2_CONFIG_DATA);
    }

    uint64_t pci_csam2_readConfig64(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
        uint32_t low = pci_csam2_readConfig32(bus, slot, func, offset);
        uint32_t high = pci_csam2_readConfig32(bus, slot, func, offset + 4);

        return ((uint64_t)high << 32) | low;
    }

    uint16_t pci_csam2_readConfig16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
        return (uint16_t)((pci_csam2_readConfig32(bus, slot, func, offset) >> (16 - ((offset & 2) << 3))) & 0xffff);
    }

    uint8_t pci_csam2_readConfig8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
        return (uint8_t)((pci_csam2_readConfig32(bus, slot, func, offset) >> (24 - ((offset & 3) << 3))) & 0xff);
    }

    void pci_csam2_writeConfig32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
        uint32_t address;
        uint32_t lbus = (uint32_t)bus;
        uint32_t lslot = (uint32_t)slot;
        uint32_t lfunc = (uint32_t)func;

        address = (lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | 0x80000000;

        outl(PCI_CSAM2_CONFIG_ADDRESS, address);
        outl(PCI_CSAM2_CONFIG_DATA, value);
    }

    void pci_csam2_writeConfig64(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint64_t value) {
        uint32_t low = value & 0xffffffff;
        uint32_t high = (value >> 32) & 0xffffffff;

        pci_csam2_writeConfig32(bus, slot, func, offset, low);
        pci_csam2_writeConfig32(bus, slot, func, offset + 4, high);
    }

    void pci_csam2_writeConfig16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value) {
        uint32_t address;
        uint32_t lbus = (uint32_t)bus;
        uint32_t lslot = (uint32_t)slot;
        uint32_t lfunc = (uint32_t)func;

        address = (lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfe) | 0x80000000;

        outl(PCI_CSAM2_CONFIG_ADDRESS, address);
        outw(PCI_CSAM2_CONFIG_DATA, value);
    }

    void pci_csam2_writeConfig8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t value) {
        uint32_t address;
        uint32_t lbus = (uint32_t)bus;
        uint32_t lslot = (uint32_t)slot;
        uint32_t lfunc = (uint32_t)func;

        address = (lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xff) | 0x80000000;

        outl(PCI_CSAM2_CONFIG_ADDRESS, address);
        outb(PCI_CSAM2_CONFIG_DATA, value);
    }

    uint8_t pci_csam2_getHeaderType(uint8_t bus, uint8_t slot, uint8_t function) {
        return pci_csam2_readConfig8(bus, slot, function, 0x0d);
    }

    uint64_t pci_csam2_detectSpace(uint8_t bus, uint8_t slot, uint8_t function, int barIndex) {
        uint64_t bar_value = pci_csam2_readConfig64(bus, slot, function, 0x10 + 4 * barIndex);
        uint64_t bar_value_new = 0;
        uint64_t bar_base_new = 0;
        pci_barType_t barType = (pci_barType_t)(bar_value & 1);
        int memorySpaceBarType = (bar_value & 0x00000006) >> 1;
        uint64_t maxLength = 0x8000000000000000;

        if(barType == PCI_BARTYPE_IO) {
            maxLength = 0x10000;

            pci_csam2_writeConfig32(bus, slot, function, 0x10 + 4 * barIndex, ((maxLength - 1) & 0xfffffffc) | (bar_value & 0x00000003));
            bar_value_new = 0xffffffff00000000 | pci_csam2_readConfig32(bus, slot, function, 0x10 + 4 * barIndex);
            pci_csam2_writeConfig32(bus, slot, function, 0x10 + 4 * barIndex, bar_value);

            bar_base_new = (bar_value_new & 0xfffffffffffffff0);
        } else {
            if(memorySpaceBarType == 0) {
                maxLength = 0x0000000100000000;
                pci_csam2_writeConfig32(bus, slot, function, 0x10 + 4 * barIndex, ((maxLength - 1) & 0xfffffff0) | (bar_value & 0x0000000f));
                bar_value_new = 0xffffffff00000000 | pci_csam2_readConfig32(bus, slot, function, 0x10 + 4 * barIndex);
                pci_csam2_writeConfig32(bus, slot, function, 0x10 + 4 * barIndex, bar_value);
            } else if(memorySpaceBarType == 1) {
                pci_csam2_writeConfig32(bus, slot, function, 0x10 + 4 * barIndex, ((maxLength - 1) & 0xfffffff0) | (bar_value & 0x0000000f));
                bar_value_new = 0xffffffff00000000 | pci_csam2_readConfig32(bus, slot, function, 0x10 + 4 * barIndex);
                pci_csam2_writeConfig32(bus, slot, function, 0x10 + 4 * barIndex, bar_value);
            } else if(memorySpaceBarType == 2) {
                pci_csam2_writeConfig64(bus, slot, function, 0x10 + 4 * barIndex, ((maxLength - 1) & 0xfffffffffffffff0) | (bar_value & 0x000000000000000f));
                bar_value_new = pci_csam2_readConfig64(bus, slot, function, 0x10 + 4 * barIndex);
                pci_csam2_writeConfig64(bus, slot, function, 0x10 + 4 * barIndex, bar_value);
            }

            bar_base_new = (bar_value_new & 0xfffffffffffffff0);
        }

        return (~bar_base_new + 1) & (maxLength - 1);
    }

    void pci_csam2_printDevice(uint8_t bus, uint8_t slot, uint8_t function) {
        char buffer[100];

        debug("PCI device found at ");
        debug(std::hex8(bus, buffer));
        debug(":");
        debug(std::hex8(slot, buffer));
        debug(".");
        debug(std::itoa(function, buffer, 10));
        debug(" ven=0x");
        debug(std::hex16(pci_csam2_readConfig16(bus, slot, function, 2), buffer));
        debug(" dev=0x");
        debug(std::hex16(pci_csam2_readConfig16(bus, slot, function, 0), buffer));
        debug(" class=0x");
        debug(std::hex8(pci_csam2_readConfig8(bus, slot, function, 8), buffer));
        debug(" sub=0x");
        debug(std::hex8(pci_csam2_readConfig8(bus, slot, function, 9), buffer));
        debug(" pif=0x");
        debug(std::hex8(pci_csam2_readConfig8(bus, slot, function, 10), buffer));
        debug("\n");

        if(pci_csam2_readConfig16(bus, slot, function, 2) == 0x10ec && pci_csam2_readConfig16(bus, slot, function, 0) == 0x8139) {
            RTL8139 networkCard(bus, slot, function);

            networkAddress_t macaddr;
            networkCard.getMACAddress(&macaddr);

            debug("RTL8139 MAC address: ");

            for(int i = 0; i < 5; i++) {
                debug(std::hex8(macaddr.addr.mac[i], buffer));
                debug(":");
            }

            debug(std::hex8(macaddr.addr.mac[5], buffer));
            debug("\n");
        }

        for(int i = 0; i < 6; i++) {
            uint64_t bar_value = pci_csam2_readConfig64(bus, slot, function, 0x10 + 4 * i);

            if(!(bar_value & 0x00000000ffffffff)) {
                continue;
            }

            uint64_t bar_base = bar_value;
            pci_barType_t barType = (pci_barType_t)(bar_value & 1);
            int memorySpaceBarType = (bar_value & 0x00000006) >> 1;
            int prefetchable = (bar_value & 0x00000008) >> 3;

            debug(" - BAR: type=");

            if(barType == PCI_BARTYPE_MMIO) {
                if(memorySpaceBarType == 0) {
                    bar_base &= 0x00000000fffffff0;
                } else if(memorySpaceBarType == 1) {
                    bar_base &= 0x00000000000ffff0;
                } else if(memorySpaceBarType == 2) {
                    bar_base &= 0xfffffffffffffff0;
                }

                debug("mmio");
            } else {
                bar_base &= 0x000000000000fffc;
                debug("io");
            }

            debug(" base=0x");

            if(barType == PCI_BARTYPE_MMIO) {
                debug(std::hex64(bar_base, buffer));
                debug(" t=");

                if(memorySpaceBarType == 0) {
                    debug("32");
                } else if(memorySpaceBarType == 1) {
                    debug("16");
                } else if(memorySpaceBarType == 2) {
                    debug("64");
                    i++;
                } else if(memorySpaceBarType == 3) {
                    debug("??");
                }

                debug(" pf=");

                if(prefetchable) {
                    debug("1");
                } else {
                    debug("0");
                }
            } else {
                debug(std::hex16(bar_base, buffer));
            }

            debug(" length=0x");
            debug(std::hex64(pci_csam2_detectSpace(bus, slot, function, i), buffer));

            debug("\n");
        }
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
    
    void PCIDevice::irqHandlerWrapper(PCIDevice *dev) {
        dev->irqHandler();
    }

    PCIDevice::PCIDevice(uint8_t bus, uint8_t slot, uint8_t func) {
        this->bus = bus;
        this->slot = slot;
        this->func = func;
        this->vendor = pci_csam2_readConfig16(bus, slot, func, 2);
        this->deviceID = pci_csam2_readConfig16(bus, slot, func, 0);
        this->_class = pci_csam2_readConfig16(bus, slot, func, 8);
        this->subclass = pci_csam2_readConfig8(bus, slot, func, 9);
        this->pif = pci_csam2_readConfig8(bus, slot, func, 10);
        this->revisionID = pci_csam2_readConfig8(bus, slot, func, 11);
        this->headerType = (pci_headerType_t)pci_csam2_readConfig8(bus, slot, func, 13);

        std::memset(this->bar, 0, sizeof(this->bar));

        for(int i = 0; i < 6; i++) {
            uint64_t bar_value = pci_csam2_readConfig64(bus, slot, func, 0x10 + 4 * i);

            if(!(bar_value & 0x00000000ffffffff)) {
                continue;
            }

            uint64_t bar_base = bar_value;
            this->bar[i].valid = true;
            this->bar[i].type = (pci_barType_t)(bar_value & 1);
            int memorySpaceBarType = (bar_value & 0x00000006) >> 1;

            if(this->bar[i].type == PCI_BARTYPE_MMIO) {
                if(memorySpaceBarType == 0) {
                    bar_base &= 0x00000000fffffff0;
                } else if(memorySpaceBarType == 1) {
                    bar_base &= 0x00000000000ffff0;
                } else if(memorySpaceBarType == 2) {
                    bar_base &= 0xfffffffffffffff0;
                }
            } else {
                bar_base &= 0x000000000000fffc;
            }

            this->bar[i].base = bar_base;

            if(memorySpaceBarType == 2) {
                i++;
            }
        }

        uint8_t irqLine = pci_csam2_readConfig8(bus, slot, func, 0x3f);

        if(irqLine != 0xff) {
            char buffer[3];
            debug("PCI interrupt line: 0x");
            debug(std::hex8(irqLine, buffer));
            debug("\n");

            // Register interrupt
            irq_register(irqLine & 0x0f, (void (*)(void *))&PCIDevice::irqHandlerWrapper, this);
        }
    }

    PCIDevice::~PCIDevice() {
        
    }

    uint8_t PCIDevice::getBus() {
        return this->bus;
    }

    uint8_t PCIDevice::getSlot() {
        return this->slot;
    }

    uint8_t PCIDevice::getFunction() {
        return this->func;
    }

    uint16_t PCIDevice::getVendorID() {
        return this->vendor;
    }
    
    uint16_t PCIDevice::getDeviceID() {
        return this->deviceID;
    }
    
    uint8_t PCIDevice::getClass() {
        return this->_class;
    }
    
    uint8_t PCIDevice::getSubclass() {
        return this->subclass;
    }
    
    uint8_t PCIDevice::getProgIF() {
        return this->pif;
    }
    
    uint8_t PCIDevice::getRevisionID() {
        return this->revisionID;
    }
    
    pci_headerType_t PCIDevice::getHeaderType() {
        return this->headerType;
    }
}
