#include <stdint.h>

#include "arch/i686/gdt.h"
#include "arch/i686/tss.h"

#include "libk/string.h"

tss_t tss;

extern int kernel_stack_bottom;

void tss_init() {
    // Prepare TSS entry in GDT
    uint32_t base = (uint32_t)&tss;
    uint16_t limit = sizeof(tss_t);

    gdt_entry_t *gdt_entry = &gdt[GDT_ENTRY_TSS];
    gdt_entry->limit0_15 = limit & 0xffff;
    gdt_entry->base0_23 = base & 0xffffff;
    gdt_entry->accessed = 1;
    gdt_entry->readWrite = 0;
    gdt_entry->direction = 0;
    gdt_entry->executable = 1; // TSS: 1 = 32 bits, 0 = 16 bits
    gdt_entry->descriptorType = 0; // 0 = TSS/LDT, 1 = GDT
    gdt_entry->privilege = 3;
    gdt_entry->present = 1;
    gdt_entry->limit16_19 = limit >> 16;
    gdt_entry->available = 0;
    gdt_entry->reserved = 0;
    gdt_entry->size = 0;
    gdt_entry->granularity = 0;
    gdt_entry->base24_31 = base >> 24;

    // Initialize TSS
    memset(&tss, 0, sizeof(tss_t));

    tss.ss0 = GDT_ENTRY_DATA32_R0 << 3;
    tss.esp0 = (uint32_t)&kernel_stack_bottom;
}
