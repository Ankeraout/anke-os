#ifndef __KERNEL_ARCH_I686_PCI_H__
#define __KERNEL_ARCH_I686_PCI_H__

#include <stddef.h>
#include <stdint.h>

typedef enum {
    PCI_BAR_BUS_IO,
    PCI_BAR_BUS_MMIO
} pci_bar_bus_t;

typedef struct {
    uint64_t base;
    pci_bar_bus_t bus;
} pci_bar_t;

typedef union {
    struct {
        uint8_t bus;
        uint8_t slot;
        uint8_t func;
    } csam1;

    struct {
        uint8_t deviceNumber;
    } csam2;

    pci_bar_t bar[6];
    int barCount;
} pci_dev_t;

void pci_init();
uint8_t pci_csam_read8(const pci_dev_t *dev, uint8_t offset);
uint16_t pci_csam_read16(const pci_dev_t *dev, uint8_t offset);
uint32_t pci_csam_read32(const pci_dev_t *dev, uint8_t offset);
void pci_csam_write8(const pci_dev_t *dev, uint8_t offset, uint8_t value);
void pci_csam_write16(const pci_dev_t *dev, uint8_t offset, uint16_t value);
void pci_csam_write32(const pci_dev_t *dev, uint8_t offset, uint32_t value);
uint8_t pci_bar_read8(const pci_bar_t *bar, size_t offset);
uint16_t pci_bar_read16(const pci_bar_t *bar, size_t offset);
uint32_t pci_bar_read32(const pci_bar_t *bar, size_t offset);
uint64_t pci_bar_getSize(const pci_bar_t *bar);

#endif
