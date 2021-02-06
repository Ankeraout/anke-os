#ifndef __KERNEL_ARCH_I686_DEV_IDE_H__
#define __KERNEL_ARCH_I686_DEV_IDE_H__

#include <stdbool.h>
#include <stdint.h>

#include "arch/i686/dev/ata.h"

typedef struct {
    ata_channel_t channels[2];
    uint16_t busMasterPort;
} ide_controller_t;

void ide_init(const pci_dev_t *dev, ide_controller_t *controller);

#endif
