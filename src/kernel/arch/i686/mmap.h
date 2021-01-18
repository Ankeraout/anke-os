#ifndef __KERNEL_ARCH_I686_MMAP_H__
#define __KERNEL_ARCH_I686_MMAP_H__

#include <stdint.h>

typedef struct {
    union {
        uint64_t base;

        struct {
            uint32_t base_low;
            uint32_t base_high;
        } __attribute__((packed));
    };

    union {
        uint64_t length;

        struct {
            uint32_t length_low;
            uint32_t length_high;
        } __attribute__((packed));
    };

    uint32_t type;
} __attribute__((packed)) mmap_entry_t;

void mmap_init();
const mmap_entry_t *mmap_get();
int mmap_getLength();

#endif
