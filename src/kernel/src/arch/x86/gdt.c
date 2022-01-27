// =============================================================================
// File inclusion
// =============================================================================
#include <stdint.h>

#include "arch/x86/asm.h"
#include "arch/x86/gdt.h"

// =============================================================================
// Private constants declaration
// =============================================================================
#define C_ACCESS_FLAG_PRESENT 0x80
#define C_ACCESS_PRIVILEGE_R0 0x00
#define C_ACCESS_PRIVILEGE_R1 0x20
#define C_ACCESS_PRIVILEGE_R2 0x40
#define C_ACCESS_PRIVILEGE_R3 0x60
#define C_ACCESS_FLAG_SEGMENT 0x10
#define C_ACCESS_FLAG_EXECUTABLE 0x08
#define C_ACCESS_FLAG_DIRECTION 0x04
#define C_ACCESS_FLAG_READ_WRITE 0x02
#define C_ACCESS_FLAG_ACCESSED 0x01

#define C_FLAG_GRANULARITY 0x8
#define C_FLAG_SIZE 0x4
#define C_FLAG_LONG 0x2

enum {
    E_GDT_INDEX_NULL,
    E_GDT_INDEX_CODE_SEGMENT,
    E_GDT_INDEX_DATA_SEGMENT
};

// =============================================================================
// Private types declaration
// =============================================================================
typedef struct {
    uint64_t limit0_15 : 16;
    uint64_t base0_23 : 24;
    uint64_t access : 8;
    uint64_t limit16_19 : 4;
    uint64_t flags : 4;
    uint64_t base24_31 : 8;
} __attribute__((packed)) t_gdtEntry;

// =============================================================================
// Private functions declaration
// =============================================================================
static void gdtInitEntry(
    t_gdtEntry *p_entry,
    uint32_t p_base,
    uint32_t p_limit,
    uint8_t p_access,
    uint8_t p_flags
);

// =============================================================================
// Private variables declaration
// =============================================================================
t_gdtEntry s_gdt[3];

// =============================================================================
// Public functions definition
// =============================================================================
void gdtInit(void) {
    // Initialize null GDT entry
    gdtInitEntry(
        &s_gdt[E_GDT_INDEX_NULL],
        0,
        0,
        0,
        0
    );

    // Initialize code segment GDT entry
    gdtInitEntry(
        &s_gdt[E_GDT_INDEX_CODE_SEGMENT],
        0x00000000,
        0xfffff,
        C_ACCESS_FLAG_PRESENT
        | C_ACCESS_PRIVILEGE_R0
        | C_ACCESS_FLAG_SEGMENT
        | C_ACCESS_FLAG_EXECUTABLE
        | C_ACCESS_FLAG_READ_WRITE,
        C_FLAG_GRANULARITY
        | C_FLAG_SIZE
    );

    // Initialize data segment GDT entry
    gdtInitEntry(
        &s_gdt[E_GDT_INDEX_DATA_SEGMENT],
        0x00000000,
        0xfffff,
        C_ACCESS_FLAG_PRESENT
        | C_ACCESS_PRIVILEGE_R0
        | C_ACCESS_FLAG_SEGMENT
        | C_ACCESS_FLAG_READ_WRITE,
        C_FLAG_GRANULARITY
        | C_FLAG_SIZE
    );

    // Reload GDTR
    lgdt(s_gdt, sizeof(s_gdt) - 1);

    // Reload data segment descriptors
    asm(
        "movw $0x10, %ax \n \
        movw %ax, %ds \n \
        movw %ax, %es \n \
        movw %ax, %fs \n \
        movw %ax, %gs \n"
    );

    // Reload code segment descriptor
    asm(
        "ljmp $0x08, $gdtInitReloadCodeSegment \n \
        gdtInitReloadCodeSegment: \n"
    );
}

// =============================================================================
// Private functions definition
// =============================================================================
static void gdtInitEntry(
    t_gdtEntry *p_entry,
    uint32_t p_base,
    uint32_t p_limit,
    uint8_t p_access,
    uint8_t p_flags
) {
    p_entry->base0_23 = p_base & 0x00ffffff;
    p_entry->base24_31 = (p_base & 0xff000000) >> 24;
    p_entry->limit0_15 = p_limit & 0x0000ffff;
    p_entry->limit16_19 = (p_limit & 0x000f0000) >> 16;
    p_entry->flags = p_flags & 0x0f;
    p_entry->access = p_access;
}
