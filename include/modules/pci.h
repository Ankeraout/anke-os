#ifndef __INCLUDE_MODULES_PCI_H__
#define __INCLUDE_MODULES_PCI_H__

#include <stdint.h>

enum {
    E_IOCTL_PCI_SCAN = 1,
    E_IOCTL_PCI_IDENTIFY
};

typedef void tf_pciScanCallback(uint8_t p_bus, uint8_t p_slot, uint8_t p_func);

struct ts_pciRequestScan {
    tf_pciScanCallback *a_callback;
};

struct ts_pciRequestIdentification {
    // Input
    uint8_t a_bus;
    uint8_t a_slot;
    uint8_t a_function;

    // Output
    uint16_t a_vendor;
    uint16_t a_device;
    uint8_t a_class;
    uint8_t a_subclass;
    uint8_t a_programmingInterface;
    uint8_t a_revision;
};

#endif
