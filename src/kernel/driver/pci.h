#ifndef __PCI_H__
#define __PCI_H__

typedef enum {
    PCI_CSAM_1 = 1,
    PCI_CSAM_2 = 2
} pci_csam_t;

int pci_init(pci_csam_t csam);

#endif
