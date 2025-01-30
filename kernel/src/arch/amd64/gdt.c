#include <stdint.h>

#include "arch/amd64/asm.h"
#include "printk.h"

struct ts_gdtEntry {
    uint64_t m_limit_0_15 : 16;
    uint64_t m_base_0_23 : 24;
    uint64_t m_access : 8;
    uint64_t m_limit_16_19 : 4;
    uint64_t m_flags : 4;
    uint64_t m_base_24_31 : 8;
} __attribute__((packed));

static void gdtInitEntry(
    struct ts_gdtEntry *p_entry,
    uint32_t p_base,
    uint32_t p_limit,
    uint8_t p_access,
    uint8_t p_flags
);

static __attribute__((aligned(sizeof(struct ts_gdtEntry)))) struct ts_gdtEntry
    s_gdt[5];

void gdtInit(void) {
    gdtInitEntry(&s_gdt[0], 0, 0, 0, 0); // Null entry
    gdtInitEntry(&s_gdt[1], 0, 0, 0x9a, 0x02); // 64-bit kernel code
    gdtInitEntry(&s_gdt[2], 0, 0, 0x92, 0x02); // 64-bit kernel data
    gdtInitEntry(&s_gdt[3], 0, 0, 0xfa, 0x02); // 64-bit user code
    gdtInitEntry(&s_gdt[4], 0, 0, 0xf2, 0x02); // 64-bit user data

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

    pr_info("gdt: GDT loaded.\n");
}

static void gdtInitEntry(
    struct ts_gdtEntry *p_entry,
    uint32_t p_base,
    uint32_t p_limit,
    uint8_t p_access,
    uint8_t p_flags
) {
    p_entry->m_base_0_23 = p_base;
    p_entry->m_base_24_31 = p_base >> 24;
    p_entry->m_limit_0_15 = p_limit;
    p_entry->m_limit_16_19 = p_limit >> 16;
    p_entry->m_access = p_access;
    p_entry->m_flags = p_flags;
}
