#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/i686/io.hpp"
#include "arch/i686/isr.hpp"
#include "syscall.hpp"

#define INSTALL_IRQ(n) idt_initEntry(&idt[32 + n], irq_handler_##n, true, 3, false, GATE_INT32, 0x08)

extern "C" void syscall_wrapper();

namespace kernel {
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
    idt_entry_t idt[256];

    void idt_initEntry(idt_entry_t *entry, void interruptHandler(), bool present, int dpl, bool storageSegment, int gateType, uint16_t codeSegmentSelector) {
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

    void idt_init() {
        // CPU exceptions
        for(int i = 0; i < 32; i++) {
            idt_initEntry(&idt[i], isr_handler_exception, true, 3, false, GATE_TRP32, 0x08);
        }

        // IRQs
        INSTALL_IRQ(0);
        INSTALL_IRQ(1);
        INSTALL_IRQ(2);
        INSTALL_IRQ(3);
        INSTALL_IRQ(4);
        INSTALL_IRQ(5);
        INSTALL_IRQ(6);
        INSTALL_IRQ(7);
        INSTALL_IRQ(8);
        INSTALL_IRQ(9);
        INSTALL_IRQ(10);
        INSTALL_IRQ(11);
        INSTALL_IRQ(12);
        INSTALL_IRQ(13);
        INSTALL_IRQ(14);
        INSTALL_IRQ(15);
        
        // Empty entries
        for(int i = 48; i < 256; i++) {
            idt_initEntry(&idt[i], NULL, false, 0, 0, 0, 0);
        }

        // Kernel syscall
        idt_initEntry(&idt[0x80], syscall_wrapper, true, 3, false, GATE_TRP32, 0x08);

        // Load IDT
        lidt(idt, sizeof(idt) - 1);
    }
}