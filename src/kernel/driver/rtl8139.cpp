#include <stdint.h>

#include "debug.hpp"
#include "arch/i686/io.hpp"
#include "driver/pci.hpp"
#include "driver/rtl8139.hpp"
#include "libk/libk.hpp"
#include "mm/vmm.hpp"

namespace kernel {
    uint8_t RTL8139::readReg8(uint8_t offset) {
        if(!this->barFound) {
            return 0xff;
        }

        switch(this->usedBar.type) {
            case PCI_BARTYPE_IO:
                return inb(this->usedBar.base + offset);
            case PCI_BARTYPE_MMIO:
                return *((uint8_t *)(this->usedBar.base + offset));
            default:
                return 0xff;
        }
    }

    uint16_t RTL8139::readReg16(uint8_t offset) {
        if(!this->barFound) {
            return 0xffff;
        }

        switch(this->usedBar.type) {
            case PCI_BARTYPE_IO:
                return inw(this->usedBar.base + offset);
            case PCI_BARTYPE_MMIO:
                return *((uint16_t *)(this->usedBar.base + offset));
            default:
                return 0xffff;
        }
    }

    uint32_t RTL8139::readReg32(uint8_t offset) {
        if(!this->barFound) {
            return 0xffffffff;
        }

        switch(this->usedBar.type) {
            case PCI_BARTYPE_IO:
                return inl(this->usedBar.base + offset);
            case PCI_BARTYPE_MMIO:
                return *((uint32_t *)(this->usedBar.base + offset));
            default:
                return 0xffffffff;
        }
    }

    void RTL8139::writeReg8(uint8_t offset, uint8_t value) {
        if(!this->barFound) {
            return;
        }

        switch(this->usedBar.type) {
            case PCI_BARTYPE_IO:
                outb(this->usedBar.base, value);
                break;
            case PCI_BARTYPE_MMIO:
                *((uint8_t *)(this->usedBar.base + offset)) = value;
                break;
        }
    }

    void RTL8139::writeReg16(uint8_t offset, uint16_t value) {
        if(!this->barFound) {
            return;
        }

        switch(this->usedBar.type) {
            case PCI_BARTYPE_IO:
                outw(this->usedBar.base, value);
                break;
            case PCI_BARTYPE_MMIO:
                *((uint16_t *)(this->usedBar.base + offset)) = value;
                break;
        }
    }

    void RTL8139::writeReg32(uint8_t offset, uint32_t value) {
        if(!this->barFound) {
            return;
        }

        switch(this->usedBar.type) {
            case PCI_BARTYPE_IO:
                outl(this->usedBar.base, value);
                break;
            case PCI_BARTYPE_MMIO:
                *((uint32_t *)(this->usedBar.base + offset)) = value;
                break;
        }
    }

    RTL8139::RTL8139(uint8_t bus, uint8_t slot, uint8_t func) : EthernetDevice(), PCIDevice(bus, slot, func) {
        this->barFound = false;

        for(int i = 0; i < 6; i++) {
            if(this->bar[i].valid) {
                if(!this->barFound|| (this->usedBar.type == PCI_BARTYPE_IO && this->bar[i].type == PCI_BARTYPE_MMIO)) {
                    this->usedBar = this->bar[i];
                    this->barFound = true;
                }
            }
        }

        if(!this->barFound) {
            debug("No BAR detected.\n");
            return;
        } else if(this->usedBar.type == PCI_BARTYPE_IO) {
            debug("Using IO to access RTL8139\n");
        } else if(this->usedBar.type == PCI_BARTYPE_MMIO) {
            debug("Using MMIO to access RTL8139\n");

            void *ptr = vmm_map((const void *)this->usedBar.base, 1, true);

            if(ptr) {
                this->usedBar.base = (uint64_t)ptr;
                debug("MMIO for RTL8139 mapped at address 0x");
                char buffer[17];
                debug(std::hex32(this->usedBar.base, buffer));
                debug(".\n");
            } else {
                debug("Failed to map MMIO for RTL8139.\n");
            }
        }

        this->reset();
    }

    RTL8139::~RTL8139() {
        if(this->barFound) {
            if(this->usedBar.type == PCI_BARTYPE_MMIO) {
                vmm_unmap((const void *)this->usedBar.base);
            }
        }
    }

    networkAddress_t *RTL8139::getMACAddress(networkAddress_t *networkAddress) {
        uint16_t io_base = this->bar[0].base;

        networkAddress->type = NETADDR_MAC;
        *((uint32_t *)&networkAddress->addr.mac[0]) = readReg32(0);
        *((uint32_t *)&networkAddress->addr.mac[4]) = readReg16(io_base + 4);

        return networkAddress;
    }

    void RTL8139::sendFrame(const void *buffer, const size_t length) {

    }

    void RTL8139::receiveFrame(const void *buffer, const size_t length) {

    }

    void RTL8139::reset() {
        if(!this->barFound) {
            return;
        }

        this->writeReg8(0x52, 0x00); // Power on
        this->writeReg8(0x37, 0x10); // Reset

        // Wait until reset is done
        while(this->readReg8(0x37) & 0x10);

        // Configure IMR
        this->writeReg16(0x3c, 0x0005);

        // Configure Rx buffer
        this->writeReg32(0x44, 0x0000008f);

        // Enable receiver and transmitter
        this->writeReg8(0x37, 0x0c);
    }
}
