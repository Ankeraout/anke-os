#ifndef __GDT_H__
#define __GDT_H__

enum {
    GDT_ENTRY_NULL,
    GDT_ENTRY_CODE32_R0,
    GDT_ENTRY_DATA32_R0,
    GDT_ENTRY_CODE32_R3,
    GDT_ENTRY_DATA32_R3,
    GDT_ENTRY_TSS,
    GDT_ENTRY_CODE16,
    GDT_ENTRY_DATA16
};

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint64_t limit0_15 : 16;
    uint64_t base0_23 : 24;
    uint64_t accessed : 1;
    uint64_t readWrite : 1;
    uint64_t direction : 1;
    uint64_t executable : 1;
    uint64_t descriptorType : 1;
    uint64_t privilege : 2;
    uint64_t present : 1;
    uint64_t limit16_19 : 4;
    uint64_t available : 1;
    uint64_t reserved : 1;
    uint64_t size : 1;
    uint64_t granularity : 1;
    uint64_t base24_31 : 8;
} __attribute__((packed)) gdt_entry_t;

extern gdt_entry_t kernel_gdt[8];

#endif
