#ifndef __KERNEL_ARCH_I686_BIOSCALL_H__
#define __KERNEL_ARCH_I686_BIOSCALL_H__

#include <stdint.h>

enum {
    BIOSCALL_CONTEXT_EFLAGS_CF = 0x0001,
    BIOSCALL_CONTEXT_EFLAGS_PF = 0x0004,
    BIOSCALL_CONTEXT_EFLAGS_AF = 0x0010,
    BIOSCALL_CONTEXT_EFLAGS_ZF = 0x0040,
    BIOSCALL_CONTEXT_EFLAGS_SF = 0x0080,
    BIOSCALL_CONTEXT_EFLAGS_TF = 0x0100,
    BIOSCALL_CONTEXT_EFLAGS_IF = 0x0200,
    BIOSCALL_CONTEXT_EFLAGS_DF = 0x0400,
    BIOSCALL_CONTEXT_EFLAGS_OF = 0x0800,
    BIOSCALL_CONTEXT_EFLAGS_IOPL = 0x0300,
    BIOSCALL_CONTEXT_EFLAGS_NT = 0x4000,
    BIOSCALL_CONTEXT_EFLAGS_RF = 0x00010000,
    BIOSCALL_CONTEXT_EFLAGS_VM = 0x00020000,
    BIOSCALL_CONTEXT_EFLAGS_AC = 0x00040000,
    BIOSCALL_CONTEXT_EFLAGS_VIF = 0x00080000,
    BIOSCALL_CONTEXT_EFLAGS_VIP = 0x00100000,
    BIOSCALL_CONTEXT_EFLAGS_ID = 0x00200000
};

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
void bioscall(uint8_t interruptNumber, bioscall_context_t *context);

#endif
