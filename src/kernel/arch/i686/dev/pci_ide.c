#include <stdbool.h>
#include <stdint.h>

#include "arch/i686/io.h"
#include "arch/i686/pci.h"
#include "arch/i686/dev/ata.h"
#include "arch/i686/dev/pci_ide.h"
#include "libk/stdio.h"
#include "libk/stdlib.h"

void ide_init(const pci_dev_t *dev, ide_controller_t *controller);
static void ide_printChannelModeInformation(int channel, int flags);

void ide_init(const pci_dev_t *dev, ide_controller_t *controller) {
    printf("pci_ide: detected PCI IDE controller\n");
    
    uint8_t pif = pci_csam_read8(dev, 10);
    uint32_t bar0 = pci_csam_read32(dev, 0x10);
    uint32_t bar1 = pci_csam_read32(dev, 0x14);
    uint32_t bar2 = pci_csam_read32(dev, 0x18);
    uint32_t bar3 = pci_csam_read32(dev, 0x1c);
    uint32_t bar4 = pci_csam_read32(dev, 0x20);

    uint16_t primaryCommandBase = bar0 & 0xfffffffc;
    uint16_t primaryControlBase = bar1 & 0xfffffffc;
    uint16_t secondaryCommandBase = bar2 & 0xfffffffc;
    uint16_t secondaryControlBase = bar3 & 0xfffffffc;
    uint16_t busMasterBase = bar4 & 0xfffffffc;

    if(pif & 0x80) {
        printf("pci_ide: this controller supports bus mastering\n");
    } else {
        printf("pci_ide: this controller does not support bus mastering\n");
    }

    ide_printChannelModeInformation(0, pif & 0x03);
    ide_printChannelModeInformation(1, (pif & 0x0c) >> 2); 

    if(!(pif & 0x01)) {
        primaryCommandBase = 0x1f0;
        primaryControlBase = 0x3f6;
    }

    if(!(pif & 0x04)) {
        secondaryCommandBase = 0x170;
        secondaryControlBase = 0x376;
    }

    ata_init(&controller->channels[0], primaryCommandBase, primaryControlBase);
    ata_init(&controller->channels[1], secondaryCommandBase, secondaryControlBase);

    controller->busMasterPort = busMasterBase;
}

static void ide_printChannelModeInformation(int channel, int flags) {
    printf("pci_ide: channel %d ", channel);

    if(flags & 0x02) {
        printf("supports both modes and works in ");
    } else {
        printf("only supports ");
    }

    if(flags & 0x01) {
        printf("native PCI");
    } else {
        printf("compatibility");
    }

    printf(" mode.\n");
}
