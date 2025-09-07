#include <stdbool.h>
#include <stdint.h>

#include "arch/amd64/asm.h"
#include "arch/amd64/tss.h"
#include "printk.h"

#define C_GDT_SEGMENT_ENTRIES 4
#define C_GDT_TSS_ENTRIES C_TSS_COUNT
#define C_GDT_SIZE (C_GDT_SEGMENT_ENTRIES * 8 + C_GDT_TSS_ENTRIES * 16)

struct ts_gdt_entry_segment {
    uint64_t m_limit_0_15 : 16;
    uint64_t m_base_0_23 : 24;
    uint64_t m_access : 8;
    uint64_t m_limit_16_19 : 4;
    uint64_t m_flags : 4;
    uint64_t m_base_24_31 : 8;
} __attribute__((packed));

struct ts_gdt_entry_tss {
    uint32_t m_limit_0_15 : 16;
    uint32_t m_base_0_23 : 24;
    uint32_t m_type : 4;
    uint32_t m_zero : 1;
    uint32_t m_dpl : 2;
    uint32_t m_present : 1;
    uint32_t m_limit_16_19 : 4;
    uint32_t m_available : 1;
    uint32_t m_zero_2 : 2;
    uint32_t m_granularity : 1;
    uint32_t m_base_24_31 : 8;
    uint32_t m_base_32_63 : 32;
    uint32_t m_reserved : 8;
    uint32_t m_zero_3 : 5;
    uint32_t m_reserved_2 : 19;
} __attribute__((packed));

static void gdt_initSegmentEntry(
    void *p_entry,
    uint32_t p_base,
    uint32_t p_limit,
    uint8_t p_access,
    uint8_t p_flags
);
static void gdt_initTssEntry(
    void *p_entry,
    void *p_base,
    uint32_t p_limit,
    uint32_t p_type,
    bool p_present
);

static __attribute__((aligned(16))) uint8_t s_gdt[C_GDT_SIZE];

void gdt_init(void) {
    gdt_initSegmentEntry(&s_gdt[0x00], 0, 0, 0, 0); // Null entry
    gdt_initSegmentEntry(&s_gdt[0x08], 0, 0, 0x9b, 0x02); // 64-bit kernel code
    gdt_initSegmentEntry(&s_gdt[0x10], 0, 0, 0xfb, 0x00); // 64-bit user code
    gdt_initSegmentEntry(&s_gdt[0x18], 0, 0, 0, 0); // Null entry for alignment

    for(int l_tssIndex = 0; l_tssIndex < C_TSS_COUNT; l_tssIndex++) {
        gdt_initTssEntry(
            &s_gdt[0x20 + sizeof(struct ts_gdt_entry_tss) * l_tssIndex],
            &g_tss[l_tssIndex],
            sizeof(struct ts_tss) - 1,
            0x09,
            true
        );
    }

    asm_lgdt(s_gdt, sizeof(s_gdt) - 1);

    asm(
        "xorw %ax, %ax \n"
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

static void gdt_initSegmentEntry(
    void *p_entry,
    uint32_t p_base,
    uint32_t p_limit,
    uint8_t p_access,
    uint8_t p_flags
) {
    struct ts_gdt_entry_segment *l_entry =
        (struct ts_gdt_entry_segment *)p_entry;

    l_entry->m_base_0_23 = p_base;
    l_entry->m_base_24_31 = p_base >> 24;
    l_entry->m_limit_0_15 = p_limit;
    l_entry->m_limit_16_19 = p_limit >> 16;
    l_entry->m_access = p_access;
    l_entry->m_flags = p_flags;
}

static void gdt_initTssEntry(
    void *p_entry,
    void *p_base,
    uint32_t p_limit,
    uint32_t p_type,
    bool p_present
) {
    struct ts_gdt_entry_tss *l_entry =
        (struct ts_gdt_entry_tss *)p_entry;

    uint64_t l_base = (uint64_t)p_base;

    l_entry->m_limit_0_15 = p_limit;
    l_entry->m_limit_16_19 = p_limit >> 16;
    l_entry->m_base_0_23 = l_base;
    l_entry->m_base_24_31 = l_base >> 24;
    l_entry->m_base_32_63 = l_base >> 32;
    l_entry->m_type = p_type;
    l_entry->m_zero = 0;
    l_entry->m_zero_2 = 0;
    l_entry->m_zero_3 = 0;
    l_entry->m_dpl = 0;
    l_entry->m_present = p_present ? 1 : 0;
    l_entry->m_available = 0;
    l_entry->m_granularity = 0;
    l_entry->m_reserved = 0;
    l_entry->m_reserved_2 = 0;
}
