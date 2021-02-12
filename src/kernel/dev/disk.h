#ifndef __KERNEL_DEV_DISK_H__
#define __KERNEL_DEV_DISK_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint64_t lba_t;

typedef struct {
    int (*read)(dev_disk_t *device, void *buffer, lba_t lba);
    int (*write)(dev_disk_t *device, const void *buffer, lba_t lba);
} dev_disk_api_t;

typedef struct {
    dev_disk_api_t api;
    size_t blockSize;
    lba_t blockCount;
    bool writable;
    bool hotplug;
    bool inserted;
    bool partition;
    uint8_t reserved[256];
} dev_disk_t;

void disk_registerDevice(dev_disk_t *disk);

#endif
