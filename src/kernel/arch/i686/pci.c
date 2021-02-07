#include <stdbool.h>
#include <stdint.h>

#include "arch/i686/bioscall.h"
#include "arch/i686/io.h"
#include "arch/i686/pci.h"
#include "arch/i686/dev/pci_ide.h"
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
static void pci_registerDevice(const pci_dev_t *dev);
static uint8_t pci_csam1_read8(const pci_dev_t *dev, uint8_t offset);
static uint16_t pci_csam1_read16(const pci_dev_t *dev, uint8_t offset);
static uint32_t pci_csam1_read32(const pci_dev_t *dev, uint8_t offset);
static void pci_csam1_write8(const pci_dev_t *dev, uint8_t offset, uint8_t value);
static void pci_csam1_write16(const pci_dev_t *dev, uint8_t offset, uint16_t value);
static void pci_csam1_write32(const pci_dev_t *dev, uint8_t offset, uint32_t value);
static uint8_t pci_csam2_read8(const pci_dev_t *dev, uint8_t offset);
static uint16_t pci_csam2_read16(const pci_dev_t *dev, uint8_t offset);
static uint32_t pci_csam2_read32(const pci_dev_t *dev, uint8_t offset);
static void pci_csam2_write8(const pci_dev_t *dev, uint8_t offset, uint8_t value);
static void pci_csam2_write16(const pci_dev_t *dev, uint8_t offset, uint16_t value);
static void pci_csam2_write32(const pci_dev_t *dev, uint8_t offset, uint32_t value);
uint8_t pci_csam_read8(const pci_dev_t *dev, uint8_t offset);
uint16_t pci_csam_read16(const pci_dev_t *dev, uint8_t offset);
uint32_t pci_csam_read32(const pci_dev_t *dev, uint8_t offset);
void pci_csam_write8(const pci_dev_t *dev, uint8_t offset, uint8_t value);
void pci_csam_write16(const pci_dev_t *dev, uint8_t offset, uint16_t value);
void pci_csam_write32(const pci_dev_t *dev, uint8_t offset, uint32_t value);
uint64_t pci_bar_getSize(const pci_bar_t *bar);

void pci_init() {
    pci_detectCsam();

    if(pci_csam == PCI_CSAM_NONE) {
        printf("pci: unsupported configuration space access mechanism\n");
        return;
    } else if(pci_csam == PCI_CSAM_2) {
        for(int deviceNumber = 0; deviceNumber < 16; deviceNumber++) {
            pci_dev_t dev;

            dev.csam2.deviceNumber = deviceNumber;

            uint16_t vendorId = pci_csam2_read16(&dev, 0);

            if(vendorId != 0xffff) {
                pci_registerDevice(&dev);
            }
        }
    } else if(pci_csam == PCI_CSAM_1) {
        for(int bus = 0; bus <= pci_lastBusNumber; bus++) {
            for(int slot = 0; slot < 32; slot++) {
                pci_dev_t dev;

                dev.csam1.bus = bus;
                dev.csam1.slot = slot;
                dev.csam1.func = 0;

                uint16_t vendorId = pci_csam1_read16(&dev, 0);

                if(vendorId != 0xffff) {
                    bool isMultifunction = (pci_csam1_read8(&dev, 0x0d) & 0x80) != 0;
                    int maxFunction;

                    if(isMultifunction) {
                        maxFunction = 7;
                    } else {
                        maxFunction = 0;
                    }

                    for(int function = 0; function <= maxFunction; function++) {
                        dev.csam1.func = function;
                        uint16_t deviceId = pci_csam1_read16(&dev, 2);

                        if(deviceId != 0xffff) {
                            pci_registerDevice(&dev);
                        }
                    }
                }
            }
        }
    }
}

static void pci_registerDevice(const pci_dev_t *dev) {
    uint16_t vendorId = pci_csam_read16(dev, 0);
    uint16_t deviceId = pci_csam_read16(dev, 2);
    uint8_t class = pci_csam_read8(dev, 8);
    uint8_t subclass = pci_csam_read8(dev, 9);
    uint8_t pif = pci_csam_read8(dev, 10);

    if(pci_csam == PCI_CSAM_1) {
        printf("pci: %02x:%02x.%1x", dev->csam1.bus, dev->csam1.slot, dev->csam1.func);
    } else if(pci_csam == PCI_CSAM_2) {
        printf("pci: %#x", dev->csam2.deviceNumber);
    }

    printf(": ven=%#04x, dev=%#04x, c=%#02x, s=%#02x, pif=%#02x.\n", vendorId, deviceId, class, subclass, pif);

    if(class == 0x01 && subclass == 0x01) {
        ide_controller_t ide_controller;
        
        ide_init(dev, &ide_controller);
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

static uint8_t pci_csam1_read8(const pci_dev_t *dev, uint8_t offset) {
    outl(PCI_CONFIG_ADDRESS, (dev->csam1.bus << 16) | (dev->csam1.slot << 11) | (dev->csam1.func << 8) | (offset & 0xfc) | (1 << 31));
    return (inl(PCI_CONFIG_DATA) >> ((3 - (offset & 3)) << 3));
}

static uint16_t pci_csam1_read16(const pci_dev_t *dev, uint8_t offset) {
    outl(PCI_CONFIG_ADDRESS, (dev->csam1.bus << 16) | (dev->csam1.slot << 11) | (dev->csam1.func << 8) | (offset & 0xfc) | (1 << 31));
    return (inl(PCI_CONFIG_DATA) >> ((offset & 2) << 3));
}

static uint32_t pci_csam1_read32(const pci_dev_t *dev, uint8_t offset) {
    outl(PCI_CONFIG_ADDRESS, (dev->csam1.bus << 16) | (dev->csam1.slot << 11) | (dev->csam1.func << 8) | (offset & 0xfc) | (1 << 31));
    return inl(PCI_CONFIG_DATA);
}

static void pci_csam1_write8(const pci_dev_t *dev, uint8_t offset, uint8_t value) {
    // TODO
    (void)dev;
    (void)offset;
    (void)value;
}

static void pci_csam1_write16(const pci_dev_t *dev, uint8_t offset, uint16_t value) {
    // TODO
    (void)dev;
    (void)offset;
    (void)value;
}

static void pci_csam1_write32(const pci_dev_t *dev, uint8_t offset, uint32_t value) {
    // TODO
    (void)dev;
    (void)offset;
    (void)value;
}

static uint8_t pci_csam2_read8(const pci_dev_t *dev, uint8_t offset) {
    return inl(0xc000 | (dev->csam2.deviceNumber << 8) | (offset & 0xfc)) >> ((3 - (offset & 3)) << 3);
}

static uint16_t pci_csam2_read16(const pci_dev_t *dev, uint8_t offset) {
    return inl(0xc000 | (dev->csam2.deviceNumber << 8) | (offset & 0xfc)) >> ((offset & 2) << 3);
}

static uint32_t pci_csam2_read32(const pci_dev_t *dev, uint8_t offset) {
    return inl(0xc000 | (dev->csam2.deviceNumber << 8) | (offset & 0xfc));
}

static void pci_csam2_write8(const pci_dev_t *dev, uint8_t offset, uint8_t value) {
    // TODO
    (void)dev;
    (void)offset;
    (void)value;
}

static void pci_csam2_write16(const pci_dev_t *dev, uint8_t offset, uint16_t value) {
    // TODO
    (void)dev;
    (void)offset;
    (void)value;
}

static void pci_csam2_write32(const pci_dev_t *dev, uint8_t offset, uint32_t value) {
    // TODO
    (void)dev;
    (void)offset;
    (void)value;
}

uint8_t pci_csam_read8(const pci_dev_t *dev, uint8_t offset) {
    if(pci_csam == PCI_CSAM_1) {
        return pci_csam1_read8(dev, offset);
    } else if(pci_csam == PCI_CSAM_2) {
        return pci_csam2_read8(dev, offset);
    } else {
        return 0xff;
    }
}

uint16_t pci_csam_read16(const pci_dev_t *dev, uint8_t offset) {
    if(pci_csam == PCI_CSAM_1) {
        return pci_csam1_read16(dev, offset);
    } else if(pci_csam == PCI_CSAM_2) {
        return pci_csam2_read16(dev, offset);
    } else {
        return 0xffff;
    }
}

uint32_t pci_csam_read32(const pci_dev_t *dev, uint8_t offset) {
    if(pci_csam == PCI_CSAM_1) {
        return pci_csam1_read32(dev, offset);
    } else if(pci_csam == PCI_CSAM_2) {
        return pci_csam2_read32(dev, offset);
    } else {
        return 0xffffffff;
    }
}

void pci_csam_write8(const pci_dev_t *dev, uint8_t offset, uint8_t value) {
    if(pci_csam == PCI_CSAM_1) {
        pci_csam1_write8(dev, offset, value);
    } else if(pci_csam == PCI_CSAM_2) {
        pci_csam2_write8(dev, offset, value);
    }
}

void pci_csam_write16(const pci_dev_t *dev, uint8_t offset, uint16_t value) {
    if(pci_csam == PCI_CSAM_1) {
        pci_csam1_write16(dev, offset, value);
    } else if(pci_csam == PCI_CSAM_2) {
        pci_csam2_write16(dev, offset, value);
    }
}

void pci_csam_write32(const pci_dev_t *dev, uint8_t offset, uint32_t value) {
    if(pci_csam == PCI_CSAM_1) {
        pci_csam1_write32(dev, offset, value);
    } else if(pci_csam == PCI_CSAM_2) {
        pci_csam2_write32(dev, offset, value);
    }
}

uint64_t pci_bar_getSize(const pci_bar_t *bar) {
    // TODO
    (void)bar;

    return 0;
}
