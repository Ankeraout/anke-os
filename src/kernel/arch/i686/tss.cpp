#include "arch/i686/gdt.hpp"
#include "arch/i686/tss.hpp"
#include "libk/libk.hpp"

#define KERNEL_INTERRUPT_STACK_SIZE 4096

namespace kernel {
    uint8_t interruptStack[KERNEL_INTERRUPT_STACK_SIZE];

    tss_t tss;

    void tss_init() {
        gdt_entry_t *entry = (gdt_entry_t *)&kernel_gdt[GDT_ENTRY_TSS];

        uint32_t base = (uint32_t)&tss;
        uint32_t limit = sizeof(tss_t);

        entry->base0_23 = base & 0x00ffffff;
        entry->base24_31 = (base & 0xff000000) >> 24;
        entry->limit0_15 = limit & 0x0000ffff;
        entry->limit16_19 = (limit & 0x000f0000) >> 16;
        entry->accessed = 1;
        entry->readWrite = 0;
        entry->direction = 0;
        entry->executable = 1;
        entry->descriptorType = 0;
        entry->privilege = 3;
        entry->present = 1;
        entry->available = 0;
        entry->reserved = 0;
        entry->size = 0;
        entry->granularity = 0;

        std::memset(&tss, 0, sizeof(tss_t));

        tss.ss0 = 0x10;
        tss.esp0 = (uint32_t)&interruptStack[KERNEL_INTERRUPT_STACK_SIZE];
    }

    void tss_flush() {
        asm volatile(" \n \
            movw $0x2b, %ax \n \
            ltrw %ax \n \
        ");
    }
}
