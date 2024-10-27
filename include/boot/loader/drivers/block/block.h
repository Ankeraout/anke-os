#ifndef __INCLUDE_BOOT_LOADER_DRIVERS_BLOCK_BLOCK_H__
#define __INCLUDE_BOOT_LOADER_DRIVERS_BLOCK_BLOCK_H__

#include <stdbool.h>

#include "boot/loader/types.h"

struct ts_block {
    bool allocated;
    bool registered;
    char name[16];
    void *driverData;
    ssize_t (*read)(
        struct ts_block *p_block,
        lba_t p_lba,
        void *p_buffer
    );
    ssize_t (*write)(
        struct ts_block *p_block,
        lba_t p_lba,
        const void *p_buffer
    );
};

int block_init(void);
struct ts_block *block_alloc(void);
int block_register(struct ts_block *p_block);

#endif
