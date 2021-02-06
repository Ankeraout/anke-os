#ifndef __KENREL_ARCH_I686_PCI_H__
#define __KENREL_ARCH_I686_PCI_H__

#include <stdint.h>

typedef union {
    struct {
        uint8_t bus;
        uint8_t slot;
        uint8_t func;
    } csam1;

    struct {
        uint8_t deviceNumber;
    } csam2;
} pci_dev_t;

void pci_init();
uint8_t pci_csam_read8(const pci_dev_t *dev, uint8_t offset);
uint16_t pci_csam_read16(const pci_dev_t *dev, uint8_t offset);
uint32_t pci_csam_read32(const pci_dev_t *dev, uint8_t offset);

#endif
