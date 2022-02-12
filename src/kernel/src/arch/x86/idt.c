// =============================================================================
// File inclusion
// =============================================================================
#include <stdbool.h>
#include <stdint.h>

#include "arch/x86/asm.h"
#include "arch/x86/idt.h"

// =============================================================================
// Private constants declaration
// =============================================================================
#define C_IDT_NUM_ENTRIES 256

// =============================================================================
// Private types declaration
// =============================================================================
struct ts_idtEntry {
    uint64_t offset0_15 : 16;
    uint64_t segmentSelector : 16;
    uint64_t reserved : 8;
    uint64_t gateType : 4;
    uint64_t zero : 1;
    uint64_t descriptorPrivilegeLevel : 2;
    uint64_t present : 1;
    uint64_t offset16_31 : 16;
} __attribute__((packed));

enum te_idtGateType {
    E_IDT_GATE_TYPE_TASK = 0x5,
    E_IDT_GATE_TYPE_INTERRUPT_16 = 0x6,
    E_IDT_GATE_TYPE_TRAP_16 = 0x7,
    E_IDT_GATE_TYPE_INTERRUPT_32 = 0xe,
    E_IDT_GATE_TYPE_TRAP_32 = 0xf
};

// =============================================================================
// Private functions declaration
// =============================================================================
static void idtInitEntry(
    struct ts_idtEntry *p_idtEntry,
    uint32_t p_offset,
    uint16_t p_selector,
    enum te_idtGateType p_gateType,
    int p_descriptorPrivilegeLevel,
    bool p_present
);

// =============================================================================
// Private variables declaration
// =============================================================================
static struct ts_idtEntry s_idt[C_IDT_NUM_ENTRIES];

// =============================================================================
// Public functions definition
// =============================================================================
void idtInit(void) {
    // All interrupts are not present
    for(
        int l_entryIndex = 0;
        l_entryIndex < C_IDT_NUM_ENTRIES;
        l_entryIndex++
    ) {
        idtInitEntry(
            &s_idt[l_entryIndex],
            0,
            0,
            E_IDT_GATE_TYPE_INTERRUPT_32,
            0,
            false
        );
    }

    // Reload IDTR
    lidt(s_idt, sizeof(s_idt) - 1);
}

// =============================================================================
// Private functions definition
// =============================================================================
static void idtInitEntry(
    struct ts_idtEntry *p_idtEntry,
    uint32_t p_offset,
    uint16_t p_selector,
    enum te_idtGateType p_gateType,
    int p_descriptorPrivilegeLevel,
    bool p_present
) {
    p_idtEntry->offset0_15 = p_offset & 0x0000ffff;
    p_idtEntry->offset16_31 = (p_offset & 0xffff0000) >> 16;
    p_idtEntry->segmentSelector = p_selector;
    p_idtEntry->gateType = p_gateType;
    p_idtEntry->descriptorPrivilegeLevel = p_descriptorPrivilegeLevel;
    p_idtEntry->present = p_present;
    p_idtEntry->reserved = 0;
    p_idtEntry->zero = 0;
}
