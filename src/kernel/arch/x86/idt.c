#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel/arch/x86/assembly.h"
#include "kernel/arch/x86/isr.h"

// Gate types
enum {
    GATE_TSK32 = 0x5,
    GATE_INT16 = 0x6,
    GATE_TRP16 = 0x7,
    GATE_INT32 = 0xe,
    GATE_TRP32 = 0xf
};

// IDT entry
typedef struct {
    uint16_t offset0_15;
    uint16_t codeSegmentSelector;
    uint8_t zero;
    uint8_t typeAttr;
    uint16_t offset16_31;
} __attribute__((packed)) idt_entry_t;

// IDT itself
static idt_entry_t idt[256];

void idt_init();
static void idt_initEntry(idt_entry_t *entry, void interruptHandler(), bool present, int dpl, bool storageSegment, int gateType, uint16_t codeSegmentSelector);

void idt_init() {
    // CPU exceptions
    idt_initEntry(&idt[0], isr_handler_exception_0, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[1], isr_handler_exception_1, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[2], isr_handler_exception_2, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[3], isr_handler_exception_3, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[4], isr_handler_exception_4, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[5], isr_handler_exception_5, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[6], isr_handler_exception_6, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[7], isr_handler_exception_7, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[8], isr_handler_exception_8, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[9], isr_handler_exception_9, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[10], isr_handler_exception_10, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[11], isr_handler_exception_11, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[12], isr_handler_exception_12, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[13], isr_handler_exception_13, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[14], isr_handler_exception_14, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[15], isr_handler_exception_15, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[16], isr_handler_exception_16, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[17], isr_handler_exception_17, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[18], isr_handler_exception_18, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[19], isr_handler_exception_19, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[20], isr_handler_exception_20, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[21], isr_handler_exception_21, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[22], isr_handler_exception_22, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[23], isr_handler_exception_23, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[24], isr_handler_exception_24, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[25], isr_handler_exception_25, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[26], isr_handler_exception_26, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[27], isr_handler_exception_27, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[28], isr_handler_exception_28, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[29], isr_handler_exception_29, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[30], isr_handler_exception_30, true, 0, false, GATE_TRP32, 0x08);
    idt_initEntry(&idt[31], isr_handler_exception_31, true, 0, false, GATE_TRP32, 0x08);

    // IRQs
    idt_initEntry(&idt[32], irq_handler_0, true, 0, false, GATE_INT32, 0x08);
    idt_initEntry(&idt[33], irq_handler_1, true, 0, false, GATE_INT32, 0x08);
    idt_initEntry(&idt[34], irq_handler_2, true, 0, false, GATE_INT32, 0x08);
    idt_initEntry(&idt[35], irq_handler_3, true, 0, false, GATE_INT32, 0x08);
    idt_initEntry(&idt[36], irq_handler_4, true, 0, false, GATE_INT32, 0x08);
    idt_initEntry(&idt[37], irq_handler_5, true, 0, false, GATE_INT32, 0x08);
    idt_initEntry(&idt[38], irq_handler_6, true, 0, false, GATE_INT32, 0x08);
    idt_initEntry(&idt[39], irq_handler_7, true, 0, false, GATE_INT32, 0x08);
    idt_initEntry(&idt[40], irq_handler_8, true, 0, false, GATE_INT32, 0x08);
    idt_initEntry(&idt[41], irq_handler_9, true, 0, false, GATE_INT32, 0x08);
    idt_initEntry(&idt[42], irq_handler_10, true, 0, false, GATE_INT32, 0x08);
    idt_initEntry(&idt[43], irq_handler_11, true, 0, false, GATE_INT32, 0x08);
    idt_initEntry(&idt[44], irq_handler_12, true, 0, false, GATE_INT32, 0x08);
    idt_initEntry(&idt[45], irq_handler_13, true, 0, false, GATE_INT32, 0x08);
    idt_initEntry(&idt[46], irq_handler_14, true, 0, false, GATE_INT32, 0x08);
    idt_initEntry(&idt[47], irq_handler_15, true, 0, false, GATE_INT32, 0x08);

    // Empty entries
    for(int i = 48; i < 256; i++) {
        idt_initEntry(&idt[i], NULL, false, 0, 0, 0, 0);
    }

    // Load IDT
    lidt(idt, sizeof(idt) - 1);
}

static void idt_initEntry(idt_entry_t *entry, void interruptHandler(), bool present, int dpl, bool storageSegment, int gateType, uint16_t codeSegmentSelector) {
    uint32_t offset = (uint32_t)interruptHandler;

    entry->codeSegmentSelector = codeSegmentSelector;
    entry->offset0_15 = offset & 0xffff;
    entry->offset16_31 = offset >> 16;
    entry->zero = 0;
    entry->typeAttr =
        (present ? 0x80 : 0x00)
        | ((dpl & 0x03) << 5)
        | (storageSegment ? 0x10 : 0x00)
        | (gateType & 0x0f)
        ;
}