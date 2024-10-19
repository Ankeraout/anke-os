#include <stdbool.h>
#include <stdint.h>

#include "boot/loader/arch/x86/asm.h"
#include "boot/loader/arch/x86/isr.h"
#include "boot/loader/stdio.h"
#include "boot/loader/string.h"

#define C_IDT_ENTRY_COUNT 256

struct ts_idtEntry {
    uint16_t offsetLow;
    uint16_t segmentSelector;
    uint8_t reserved : 8;
    uint8_t gateType : 4;
    uint8_t zero : 1;
    uint8_t dpl : 2;
    uint8_t present : 1;
    uint16_t offsetHigh;
} __attribute__((packed));

enum te_idtGateType {
    E_IDT_GATETYPE_INT32 = 0xe,
    E_IDT_GATETYPE_TRAP32 = 0xf
};

static __attribute__((aligned(sizeof(struct ts_idtEntry))))
struct ts_idtEntry s_idt[C_IDT_ENTRY_COUNT];

static void idtInitEntry(
    struct ts_idtEntry *p_entry,
    void (*p_handler)(void),
    uint16_t p_segmentSelector,
    enum te_idtGateType p_gateType,
    int p_dpl,
    bool p_present
);

void idt_init(void) {
    // By default, all entries are absent.
    memset(&s_idt, 0, sizeof(s_idt));

    // Set exception handlers
    idtInitEntry(&s_idt[0], isr_exception0, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[1], isr_exception1, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[2], isr_exception2, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[3], isr_exception3, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[4], isr_exception4, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[5], isr_exception5, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[6], isr_exception6, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[7], isr_exception7, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[8], isr_exception8, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[9], isr_exception9, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[10], isr_exception10, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[11], isr_exception11, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[12], isr_exception12, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[13], isr_exception13, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[14], isr_exception14, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[15], isr_exception15, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[16], isr_exception16, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[17], isr_exception17, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[18], isr_exception18, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[19], isr_exception19, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[20], isr_exception20, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[21], isr_exception21, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[22], isr_exception22, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[23], isr_exception23, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[24], isr_exception24, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[25], isr_exception25, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[26], isr_exception26, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[27], isr_exception27, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[28], isr_exception28, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[29], isr_exception29, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[30], isr_exception30, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);
    idtInitEntry(&s_idt[31], isr_exception31, 0x0008, E_IDT_GATETYPE_TRAP32, 0, true);

    // Set IRQ handlers
    idtInitEntry(&s_idt[32], isr_irq32, 0x0008, E_IDT_GATETYPE_INT32, 0, true);
    idtInitEntry(&s_idt[33], isr_irq33, 0x0008, E_IDT_GATETYPE_INT32, 0, true);
    idtInitEntry(&s_idt[34], isr_irq34, 0x0008, E_IDT_GATETYPE_INT32, 0, true);
    idtInitEntry(&s_idt[35], isr_irq35, 0x0008, E_IDT_GATETYPE_INT32, 0, true);
    idtInitEntry(&s_idt[36], isr_irq36, 0x0008, E_IDT_GATETYPE_INT32, 0, true);
    idtInitEntry(&s_idt[37], isr_irq37, 0x0008, E_IDT_GATETYPE_INT32, 0, true);
    idtInitEntry(&s_idt[38], isr_irq38, 0x0008, E_IDT_GATETYPE_INT32, 0, true);
    idtInitEntry(&s_idt[39], isr_irq39, 0x0008, E_IDT_GATETYPE_INT32, 0, true);
    idtInitEntry(&s_idt[40], isr_irq40, 0x0008, E_IDT_GATETYPE_INT32, 0, true);
    idtInitEntry(&s_idt[41], isr_irq41, 0x0008, E_IDT_GATETYPE_INT32, 0, true);
    idtInitEntry(&s_idt[42], isr_irq42, 0x0008, E_IDT_GATETYPE_INT32, 0, true);
    idtInitEntry(&s_idt[43], isr_irq43, 0x0008, E_IDT_GATETYPE_INT32, 0, true);
    idtInitEntry(&s_idt[44], isr_irq44, 0x0008, E_IDT_GATETYPE_INT32, 0, true);
    idtInitEntry(&s_idt[45], isr_irq45, 0x0008, E_IDT_GATETYPE_INT32, 0, true);
    idtInitEntry(&s_idt[46], isr_irq46, 0x0008, E_IDT_GATETYPE_INT32, 0, true);
    idtInitEntry(&s_idt[47], isr_irq47, 0x0008, E_IDT_GATETYPE_INT32, 0, true);

    lidt(s_idt, sizeof(s_idt) - 1);
}

static void idtInitEntry(
    struct ts_idtEntry *p_entry,
    void (*p_handler)(void),
    uint16_t p_segmentSelector,
    enum te_idtGateType p_gateType,
    int p_dpl,
    bool p_present
) {
    uintptr_t l_handler = (uintptr_t)p_handler;

    p_entry->offsetLow = l_handler;
    p_entry->offsetHigh = l_handler >> 16;
    p_entry->segmentSelector = p_segmentSelector;
    p_entry->gateType = p_gateType;
    p_entry->dpl = p_dpl;
    p_entry->present = p_present ? 1 : 0;
    p_entry->reserved = 0;
    p_entry->zero = 0;
}
