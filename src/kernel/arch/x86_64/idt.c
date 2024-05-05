#include <stdbool.h>
#include <stdint.h>

#include "kernel/arch/x86_64/asm.h"
#include "kernel/arch/x86_64/isr.h"
#include "kernel/printk.h"

#define C_IDT_ENTRY_COUNT 256

struct ts_idtEntry {
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

enum te_idtGateType {
    E_IDT_GATETYPE_INT64 = 0xe,
    E_IDT_GATETYPE_TRAP64 = 0xf
};

static __attribute__((aligned(sizeof(struct ts_idtEntry)))) struct ts_idtEntry s_idt[C_IDT_ENTRY_COUNT];

static void idtInitEntry(
    struct ts_idtEntry *p_entry,
    void (*p_handler)(void),
    uint16_t p_segmentSelector,
    uint8_t p_interruptStackTable,
    enum te_idtGateType p_gateType,
    int p_dpl,
    bool p_present
);

void idtInit(void) {
    // By default, all entries are absent.
    for(int l_index = 0; l_index < C_IDT_ENTRY_COUNT; l_index++) {
        idtInitEntry(&s_idt[l_index], 0, 0, 0, 0, 0, false);
    }

    // Set exception handlers
    idtInitEntry(&s_idt[0], isrException0, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[1], isrException1, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[2], isrException2, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[3], isrException3, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[4], isrException4, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[5], isrException5, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[6], isrException6, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[7], isrException7, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[8], isrException8, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[9], isrException9, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[10], isrException10, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[11], isrException11, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[12], isrException12, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[13], isrException13, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[14], isrException14, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[15], isrException15, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[16], isrException16, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[17], isrException17, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[18], isrException18, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[19], isrException19, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[20], isrException20, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[21], isrException21, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[22], isrException22, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[23], isrException23, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[24], isrException24, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[25], isrException25, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[26], isrException26, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[27], isrException27, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[28], isrException28, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[29], isrException29, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[30], isrException30, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);
    idtInitEntry(&s_idt[31], isrException31, 0x0008, 0, E_IDT_GATETYPE_TRAP64, 0, true);

    // Set IRQ handlers
    idtInitEntry(&s_idt[32], isrIrq32, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idtInitEntry(&s_idt[33], isrIrq33, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idtInitEntry(&s_idt[34], isrIrq34, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idtInitEntry(&s_idt[35], isrIrq35, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idtInitEntry(&s_idt[36], isrIrq36, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idtInitEntry(&s_idt[37], isrIrq37, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idtInitEntry(&s_idt[38], isrIrq38, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idtInitEntry(&s_idt[39], isrIrq39, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idtInitEntry(&s_idt[40], isrIrq40, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idtInitEntry(&s_idt[41], isrIrq41, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idtInitEntry(&s_idt[42], isrIrq42, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idtInitEntry(&s_idt[43], isrIrq43, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idtInitEntry(&s_idt[44], isrIrq44, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idtInitEntry(&s_idt[45], isrIrq45, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idtInitEntry(&s_idt[46], isrIrq46, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);
    idtInitEntry(&s_idt[47], isrIrq47, 0x0008, 0, E_IDT_GATETYPE_INT64, 0, true);

    lidt(s_idt, sizeof(s_idt) - 1);

    printk("idt: IDT loaded.\n");
}

static void idtInitEntry(
    struct ts_idtEntry *p_entry,
    void (*p_handler)(void),
    uint16_t p_segmentSelector,
    uint8_t p_interruptStackTable,
    enum te_idtGateType p_gateType,
    int p_dpl,
    bool p_present
) {
    uintptr_t l_handler = (uintptr_t)p_handler;

    p_entry->m_offset_0_15 = l_handler;
    p_entry->m_offset_16_31 = l_handler >> 16;
    p_entry->m_offset_32_63 = l_handler >> 32;
    p_entry->m_segment = p_segmentSelector;
    p_entry->m_ist = p_interruptStackTable;
    p_entry->m_gateType = p_gateType;
    p_entry->m_dpl = p_dpl;
    p_entry->m_present = p_present ? 1 : 0;
    p_entry->m_reserved_0 = 0;
    p_entry->m_reserved_1 = 0;
    p_entry->m_zero = 0;
}
