#ifndef __KERNEL_DRIVER_PCI_HPP__
#define __KERNEL_DRIVER_PCI_HPP__

namespace kernel {
    typedef enum {
        PCI_CSAM_1 = 1,
        PCI_CSAM_2 = 2
    } pci_csam_t;

    int pci_init(pci_csam_t csam);
}

#endif
