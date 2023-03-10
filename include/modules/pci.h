#ifndef __INCLUDE_MODULES_PCI_H__
#define __INCLUDE_MODULES_PCI_H__

#include <stdint.h>

enum {
    E_IOCTL_PCI_SCAN = 1
};

typedef void tf_pciScanCallback(uint8_t p_bus, uint8_t p_slot, uint8_t p_func);

struct ts_pciRequestScan {
    tf_pciScanCallback *a_callback;
};

#endif
