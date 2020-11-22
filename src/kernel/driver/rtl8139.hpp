#ifndef __KERNEL_DRIVER_RTL8139_HPP__
#define __KERNEL_DRIVER_RTL8139_HPP__

#include <stdint.h>

#include "driver/eth.hpp"
#include "driver/pci.hpp"

namespace kernel {
    class RTL8139 : public EthernetDevice, public PCIDevice {
        private:
        bool barFound;
        pci_bar_t usedBar;
        uint8_t receiveBuffer[8192 + 16 + 1500];

        virtual uint8_t readReg8(uint8_t offset);
        virtual uint16_t readReg16(uint8_t offset);
        virtual uint32_t readReg32(uint8_t offset);
        virtual void writeReg8(uint8_t offset, uint8_t value);
        virtual void writeReg16(uint8_t offset, uint16_t value);
        virtual void writeReg32(uint8_t offset, uint32_t value);

        public:
        RTL8139(uint8_t bus, uint8_t slot, uint8_t func);
        virtual ~RTL8139();
        virtual networkAddress_t *getMACAddress(networkAddress_t *networkAddress);
        virtual void sendFrame(const void *buffer, const size_t length);
        virtual void receiveFrame(const void *buffer, const size_t length);
        virtual void reset();
    };
}

#endif
