#ifndef __KERNEL_ARCH_I686_BIOSCALL_H__
#define __KERNEL_ARCH_I686_BIOSCALL_H__

#include <stdint.h>

typedef struct {
    union {
        uint32_t edi;
        uint16_t di;
    };

    union {
        uint32_t esi;
        uint16_t si;
    };

    union {
        uint32_t ebp;
        uint16_t bp;
    };

    union {
        uint32_t ebx;
        uint16_t bx;

        struct {
            uint8_t bl;
            uint8_t bh;
        };
    };

    union {
        uint32_t edx;
        uint16_t dx;

        struct {
            uint8_t dl;
            uint8_t dh;
        };
    };

    union {
        uint32_t ecx;
        uint16_t cx;

        struct {
            uint8_t cl;
            uint8_t ch;
        };
    };

    union {
        uint32_t eax;
        uint16_t ax;

        struct {
            uint8_t al;
            uint8_t ah;
        };
    };

    uint16_t gs;
    uint16_t fs;
    uint16_t es;
    uint16_t ds;

    union {
        uint32_t eflags;
        uint16_t flags;
    };
} __attribute__((packed)) bioscall_context_t;

void bioscall_init();
void bioscall(bioscall_context_t *context, uint8_t interruptNumber);

#endif
