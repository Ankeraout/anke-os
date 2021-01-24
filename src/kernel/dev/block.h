#ifndef __KERNEL_DEV_BLOCK_H__
#define __KERNEL_DEV_BLOCK_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint64_t lba_t;

typedef struct {
    const size_t blockSize;
    const bool canWrite;

    int (*read)(void *buffer, lba_t lba);
} dev_block_api_t;

typedef struct {
    dev_block_api_t api;
    uint8_t reserved[256];
} dev_block_t;

#endif
