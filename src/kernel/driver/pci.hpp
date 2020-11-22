#ifndef __KERNEL_DRIVER_PCI_HPP__
#define __KERNEL_DRIVER_PCI_HPP__

#include <stdint.h>

namespace kernel {
    typedef enum {
        PCI_HEADERTYPE_STANDARD = 0,
        PCI_HEADERTYPE_BRIDGE_PCI2PCI = 1,
        PCI_HEADERTYPE_BRIDGE_PCI2CB = 2
    } pci_headerType_t;

    typedef enum {
        PCI_BARTYPE_MMIO = 0,
        PCI_BARTYPE_IO = 1
    } pci_barType_t;

    typedef struct {
        uint64_t base;
        pci_barType_t type;
        bool valid;
    } pci_bar_t;

    class PCIDevice {
        private:
        uint8_t bus;
        uint8_t slot;
        uint8_t func;
        uint16_t vendor;
        uint16_t deviceID;
        uint8_t _class;
        uint8_t subclass;
        uint8_t pif;
        uint8_t revisionID;
        pci_headerType_t headerType;

        protected:
        pci_bar_t bar[6];

        public:
        PCIDevice(uint8_t bus, uint8_t slot, uint8_t func);
        virtual ~PCIDevice();
        virtual uint8_t getBus();
        virtual uint8_t getSlot();
        virtual uint8_t getFunction();
        virtual uint16_t getVendorID();
        virtual uint16_t getDeviceID();
        virtual uint8_t getClass();
        virtual uint8_t getSubclass();
        virtual uint8_t getProgIF();
        virtual uint8_t getRevisionID();
        virtual pci_headerType_t getHeaderType();
        virtual void reset() = 0;
    };

    typedef enum {
        PCI_CSAM_1 = 1,
        PCI_CSAM_2 = 2
    } pci_csam_t;

    int pci_init(pci_csam_t csam);
}

#endif
