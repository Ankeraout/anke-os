#include <stdbool.h>
#include <stdint.h>

#include "arch/amd64/asm.h"
#include "arch/amd64/isr.h"
#include "printk.h"

#define C_IDT_ENTRY_COUNT 256

struct ts_idt_entry {
    uint64_t m_offset_0_15 : 16;
    uint64_t m_segment : 16;
    uint64_t m_ist : 3;
    uint64_t m_reserved_0 : 5;
    uint64_t m_gateType : 4;
    uint64_t m_zero : 1;
    uint64_t m_dpl : 2;
    uint64_t m_present : 1;
    uint64_t m_offset_16_31 : 16;
    uint64_t m_offset_32_63 : 32;
    uint64_t m_reserved_1 : 32;
} __attribute__((packed));

enum te_idt_gateType {
    E_IDT_GATETYPE_INT64 = 0xe,
    E_IDT_GATETYPE_TRAP64 = 0xf
};

static __attribute__((aligned(sizeof(struct ts_idt_entry)))) struct ts_idt_entry s_idt[C_IDT_ENTRY_COUNT];

static void idt_initEntry(
    struct ts_idt_entry *p_entry,
    void (*p_handler)(void),
    uint16_t p_segmentSelector,
    uint8_t p_interruptStackTable,
    enum te_idt_gateType p_gateType,
    int p_dpl,
    bool p_present
);

void idt_init(void) {
    // By default, all entries are absent.
    for(int l_index = 0; l_index < C_IDT_ENTRY_COUNT; l_index++) {
        idt_initEntry(&s_idt[l_index], 0, 0, 0, 0, 0, false);
    }

    // Set exception handlers
    idt_initEntry(&s_idt[0], isr_exception0, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[1], isr_exception1, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[2], isr_exception2, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[3], isr_exception3, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[4], isr_exception4, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[5], isr_exception5, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[6], isr_exception6, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[7], isr_exception7, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[8], isr_exception8, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[9], isr_exception9, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[10], isr_exception10, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[11], isr_exception11, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[12], isr_exception12, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[13], isr_exception13, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[14], isr_exception14, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[15], isr_exception15, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[16], isr_exception16, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[17], isr_exception17, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[18], isr_exception18, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[19], isr_exception19, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[20], isr_exception20, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[21], isr_exception21, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[22], isr_exception22, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[23], isr_exception23, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[24], isr_exception24, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[25], isr_exception25, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[26], isr_exception26, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[27], isr_exception27, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[28], isr_exception28, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[29], isr_exception29, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[30], isr_exception30, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idt_initEntry(&s_idt[31], isr_exception31, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);

    // Set IRQ handlers
    idt_initEntry(&s_idt[32], isr_irq0, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idt_initEntry(&s_idt[33], isr_irq1, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idt_initEntry(&s_idt[34], isr_irq2, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idt_initEntry(&s_idt[35], isr_irq3, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idt_initEntry(&s_idt[36], isr_irq4, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idt_initEntry(&s_idt[37], isr_irq5, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idt_initEntry(&s_idt[38], isr_irq6, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idt_initEntry(&s_idt[39], isr_irq7, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idt_initEntry(&s_idt[40], isr_irq8, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idt_initEntry(&s_idt[41], isr_irq9, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idt_initEntry(&s_idt[42], isr_irq10, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idt_initEntry(&s_idt[43], isr_irq11, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idt_initEntry(&s_idt[44], isr_irq12, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idt_initEntry(&s_idt[45], isr_irq13, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idt_initEntry(&s_idt[46], isr_irq14, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idt_initEntry(&s_idt[47], isr_irq15, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);

    asm_lidt(s_idt, sizeof(s_idt) - 1);

    pr_info("idt: IDT loaded.\n");
}

static void idt_initEntry(
    struct ts_idt_entry *p_entry,
    void (*p_handler)(void),
    uint16_t p_segmentSelector,
    uint8_t p_interruptStackTable,
    enum te_idt_gateType p_gateType,
    int p_dpl,
    bool p_present
) {
    uintptr_t l_handler = (uintptr_t)p_handler;

    p_entry->m_offset_0_15 = l_handler;
    p_entry->m_offset_16_31 = l_handler >> 16UL;
    p_entry->m_offset_32_63 = l_handler >> 32UL;
    p_entry->m_segment = p_segmentSelector;
    p_entry->m_ist = p_interruptStackTable;
    p_entry->m_gateType = p_gateType;
    p_entry->m_dpl = p_dpl;
    p_entry->m_present = p_present ? 1UL : 0UL;
    p_entry->m_reserved_0 = 0UL;
    p_entry->m_reserved_1 = 0UL;
    p_entry->m_zero = 0UL;
}
