#include <stdint.h>

#include "kernel/arch/x86_64/inline.h"
#include "klibc/debug.h"

struct ts_gdtEntry {
    uint16_t a_limitLow;
    uint16_t a_baseLow;
    uint8_t a_baseMid;
    uint8_t a_access;
    uint8_t a_limitHigh : 4;
    uint8_t a_flags : 4;
    uint8_t a_baseHigh;
} __attribute__((packed));

static void gdtInitEntry(
    struct ts_gdtEntry *p_entry,
    uint32_t p_base,
    uint32_t p_limit,
    uint8_t p_access,
    uint8_t p_flags
);

static __attribute__((aligned(sizeof(struct ts_gdtEntry)))) struct ts_gdtEntry s_gdt[6];

void gdtInit(void) {
    gdtInitEntry(&s_gdt[0], 0, 0, 0, 0); // Null entry
    gdtInitEntry(&s_gdt[1], 0, 0, 0x9a, 0x02); // 64-bit kernel code
    gdtInitEntry(&s_gdt[2], 0, 0, 0x92, 0x02); // 64-bit kernel data
    gdtInitEntry(&s_gdt[3], 0, 0, 0xfa, 0x02); // 64-bit user code
    gdtInitEntry(&s_gdt[4], 0, 0, 0xf2, 0x02); // 64-bit user data
    gdtInitEntry(&s_gdt[5], 0, 0, 0, 0); // TSS (TODO)

    lgdt(s_gdt, sizeof(s_gdt) - 1);

    asm(
        "movw $0x10, %ax \n"
        "movw %ax, %ds \n"
        "movw %ax, %es \n"
        "movw %ax, %fs \n"
        "movw %ax, %gs \n"
        "movw %ax, %ss \n"
        "pushq $0x08 \n"
        "pushq $gdtReloadCs \n"
        "retfq \n"
        "gdtReloadCs: \n"
    );

    kernelDebug("gdt: GDT loaded.\n");
}

static void gdtInitEntry(
    struct ts_gdtEntry *p_entry,
    uint32_t p_base,
    uint32_t p_limit,
    uint8_t p_access,
    uint8_t p_flags
) {
    p_entry->a_baseLow = p_base;
    p_entry->a_baseMid = p_base >> 16;
    p_entry->a_baseHigh = p_base >> 24;
    p_entry->a_limitLow = p_limit;
    p_entry->a_limitHigh = p_limit >> 16;
    p_entry->a_access = p_access;
    p_entry->a_flags = p_flags;
}
