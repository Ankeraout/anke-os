#ifndef __KERNEL_DEV_BLOCK_H__
#define __KERNEL_DEV_BLOCK_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint64_t lba_t;

typedef struct {
    int (*read)(dev_block_t *device, void *buffer, lba_t lba);
    int (*write)(dev_block_t *device, const void *buffer, size_t size, lba_t lba);
} dev_block_api_t;

typedef struct {
    dev_block_api_t api;
    const size_t blockSize;
    const bool writable;
    const bool hotplug;
    uint8_t reserved[256];
} dev_block_t;

#endif
