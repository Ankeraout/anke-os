#ifndef __KERNEL_ARCH_I686_DEV_RTL8139_H__
#define __KERNEL_ARCH_I686_DEV_RTL8139_H__

#include "arch/i686/pci.h"

int rtl8139_init(const pci_dev_t *dev);

#endif
