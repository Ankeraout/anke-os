#include <stdbool.h>
#include <stdint.h>

#include "arch/i686/bioscall.h"
#include "arch/i686/io.h"
#include "libk/stdio.h"

#define PCI_CONFIG_ADDRESS 0xcf8
#define PCI_CONFIG_DATA 0xcfc

typedef enum {
    PCI_CSAM_NONE,
    PCI_CSAM_1,
    PCI_CSAM_2
} pci_csam_t;

static pci_csam_t pci_csam;
static int pci_lastBusNumber;

void pci_init();
static void pci_detectCsam();
static uint8_t pci_csam1_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
static uint16_t pci_csam1_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
static uint32_t pci_csam1_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

void pci_init() {
    pci_detectCsam();

    if(pci_csam == PCI_CSAM_NONE) {
        return;
    } else if(pci_csam == PCI_CSAM_2) {
        return;
    } else if(pci_csam == PCI_CSAM_1) {
        for(int bus = 0; bus <= pci_lastBusNumber; bus++) {
            for(int device = 0; device < 32; device++) {
                uint16_t vendorId = pci_csam1_read16(bus, device, 0, 0);

                if(vendorId != 0xffff) {
                    bool isMultifunction = (pci_csam1_read8(bus, device, 0, 0x0d) & 0x80) != 0;
                    int maxFunction;

                    if(isMultifunction) {
                        maxFunction = 7;
                    } else {
                        maxFunction = 0;
                    }

                    for(int function = 0; function <= maxFunction; function++) {
                        uint16_t deviceId = pci_csam1_read16(bus, device, function, 2);

                        if(deviceId != 0xffff) {
                            printf("pci: found device at %02x:%02x.%1x: (vendor=%#04x, device=%#04x).\n", bus, device, function, vendorId, deviceId);
                        }
                    }
                }
            }
        }
    }
}

static void pci_detectCsam() {
    bioscall_context_t context = {
        .ax = 0xb101,
        .edi = 0x00000000
    };

    bioscall(0x1a, &context);

    // If the BIOS call succeeded:
    //  - ah = 0x00
    //  - CF is clear
    //  - edx = 0x20494350 ("PCI ")
    //  - edi = physical pointer to protected-mode entry point
    //  - al = PCI hardware characteristics
    //  - bh = PCI interface level major version
    //  - bl = PCI interface level minor version
    //  - cl = number of last PCI bus in the system
    if(context.ah != 0x00) {
        printf("pci: PCI bus detection failed (ah != 0x00).\n");
        pci_csam = PCI_CSAM_NONE;
    } else if(context.eflags & BIOSCALL_CONTEXT_EFLAGS_CF) {
        printf("pci: PCI bus detection failed (cf set).\n");
        pci_csam = PCI_CSAM_NONE;
    } else if(context.edx != 0x20494350) {
        printf("pci: PCI bus detection failed (edx != 0x20494350).\n");
        pci_csam = PCI_CSAM_NONE;
    } else {
        printf("pci: detected PCI %x.%x.\n", context.bh, context.bl);
        printf("pci: last PCI bus is bus %#02x.\n", context.cl);

        pci_lastBusNumber = context.cl;

        if(context.al & (1 << 0)) {
            printf("pci: detected configuration space access mechanism #1.\n");
            pci_csam = PCI_CSAM_1;
        } else if(context.al & (1 << 1)) {
            printf("pci: detected configuration space access mechanism #2.\n");
            pci_csam = PCI_CSAM_2;
        } else {
            printf("pci: no configuration space access mechanism detected.\n");
            pci_csam = PCI_CSAM_NONE;
        }
    }
}

static uint8_t pci_csam1_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    outl(PCI_CONFIG_ADDRESS, (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xfc) | (1 << 31));
    return (inl(PCI_CONFIG_DATA) >> ((3 - (offset & 3)) << 3));
}

static uint16_t pci_csam1_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    outl(PCI_CONFIG_ADDRESS, (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xfc) | (1 << 31));
    return (inl(PCI_CONFIG_DATA) >> ((offset & 2) << 3));
}

static uint32_t pci_csam1_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    outl(PCI_CONFIG_ADDRESS, (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xfc) | (1 << 31));
    return inl(PCI_CONFIG_DATA);
}
