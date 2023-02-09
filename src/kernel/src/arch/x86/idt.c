#include <stdbool.h>
#include <stdint.h>

#include "arch/x86/inline.h"
#include "debug.h"

#define C_IDT_ENTRY_COUNT 256

struct ts_idtEntry {
    uint16_t a_offsetLow;
    uint16_t a_segmentSelector;
    uint8_t a_interruptStackTable : 3;
    uint8_t a_reserved1 : 5;
    uint8_t a_gateType : 4;
    uint8_t a_zero : 1;
    uint8_t a_dpl : 2;
    uint8_t a_present : 1;
    uint16_t a_offsetMid;
    uint32_t a_offsetHigh;
    uint32_t a_reserved2;
} __attribute__((packed));

enum te_idtGateType {
    E_IDT_GATETYPE_INT64 = 0xe,
    E_IDT_GATETYPE_TRAP64 = 0xf
};

static __attribute__((aligned(sizeof(struct ts_idtEntry)))) struct ts_idtEntry s_idt[C_IDT_ENTRY_COUNT];

static void idtInitEntry(
    struct ts_idtEntry *p_entry,
    uint64_t p_offset,
    uint16_t p_segmentSelector,
    uint8_t p_interruptStackTable,
    enum te_idtGateType p_gateType,
    int p_dpl,
    bool p_present
) {
    p_entry->a_offsetLow = p_offset;
    p_entry->a_offsetMid = p_offset >> 16;
    p_entry->a_offsetHigh = p_offset >> 32;
    p_entry->a_segmentSelector = p_segmentSelector;
    p_entry->a_interruptStackTable = p_interruptStackTable;
    p_entry->a_gateType = p_gateType;
    p_entry->a_dpl = p_dpl;
    p_entry->a_present = p_present ? 1 : 0;
    p_entry->a_reserved1 = 0;
    p_entry->a_reserved2 = 0;
    p_entry->a_zero = 0;
}

void idtInit(void) {
    // By default, all entries are absent.
    for(int l_index = 0; l_index < C_IDT_ENTRY_COUNT; l_index++) {
        idtInitEntry(&s_idt[l_index], 0, 0, 0, 0, 0, false);
    }

    lidt(s_idt, sizeof(s_idt) - 1);

    debugPrint("idt: IDT loaded.\n");
}
