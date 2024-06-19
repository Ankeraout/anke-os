#include <stdint.h>

#include "arch/x86/asm.h"
#include "arch/x86/idt.h"

#define C_INTERRUPT_GATE 0x0e
#define C_TRAP_GATE 0x0f

struct ts_idtEntry {
    uint16_t m_offsetLow;
    uint16_t m_segment;
    uint8_t m_reserved;
    uint8_t m_gateType : 4;
    uint8_t m_zero : 1;
    uint8_t m_dpl : 2;
    uint8_t m_present : 1;
    uint16_t m_offsetHigh;
};

struct ts_idtr {
    uint16_t m_limit;
    uint32_t m_base;
} __attribute__((packed));

static void idtInitEntry(
    struct ts_idtEntry *p_entry,
    uint32_t p_offset,
    uint16_t p_segment,
    uint8_t p_gateType
);

extern void isrException0(void);
extern void isrException1(void);
extern void isrException2(void);
extern void isrException3(void);
extern void isrException4(void);
extern void isrException5(void);
extern void isrException6(void);
extern void isrException7(void);
extern void isrException8(void);
extern void isrException9(void);
extern void isrException10(void);
extern void isrException11(void);
extern void isrException12(void);
extern void isrException13(void);
extern void isrException14(void);
extern void isrException15(void);
extern void isrException16(void);
extern void isrException17(void);
extern void isrException18(void);
extern void isrException19(void);
extern void isrException20(void);
extern void isrException21(void);
extern void isrException22(void);
extern void isrException23(void);
extern void isrException24(void);
extern void isrException25(void);
extern void isrException26(void);
extern void isrException27(void);
extern void isrException28(void);
extern void isrException29(void);
extern void isrException30(void);
extern void isrException31(void);
extern void isrIrq32(void);
extern void isrIrq33(void);
extern void isrIrq34(void);
extern void isrIrq35(void);
extern void isrIrq36(void);
extern void isrIrq37(void);
extern void isrIrq38(void);
extern void isrIrq39(void);
extern void isrIrq40(void);
extern void isrIrq41(void);
extern void isrIrq42(void);
extern void isrIrq43(void);
extern void isrIrq44(void);
extern void isrIrq45(void);
extern void isrIrq46(void);
extern void isrIrq47(void);

static struct ts_idtEntry s_idt[8192];

void idtInit(void) {
    idtInitEntry(&s_idt[0], (uint32_t)isrException0, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[1], (uint32_t)isrException1, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[2], (uint32_t)isrException2, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[3], (uint32_t)isrException3, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[4], (uint32_t)isrException4, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[5], (uint32_t)isrException5, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[6], (uint32_t)isrException6, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[7], (uint32_t)isrException7, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[8], (uint32_t)isrException8, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[9], (uint32_t)isrException9, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[10], (uint32_t)isrException10, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[11], (uint32_t)isrException11, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[12], (uint32_t)isrException12, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[13], (uint32_t)isrException13, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[14], (uint32_t)isrException14, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[15], (uint32_t)isrException15, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[16], (uint32_t)isrException16, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[17], (uint32_t)isrException17, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[18], (uint32_t)isrException18, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[19], (uint32_t)isrException19, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[20], (uint32_t)isrException20, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[21], (uint32_t)isrException21, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[22], (uint32_t)isrException22, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[23], (uint32_t)isrException23, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[24], (uint32_t)isrException24, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[25], (uint32_t)isrException25, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[26], (uint32_t)isrException26, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[27], (uint32_t)isrException27, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[28], (uint32_t)isrException28, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[29], (uint32_t)isrException29, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[30], (uint32_t)isrException30, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[31], (uint32_t)isrException31, 0x08, C_TRAP_GATE);
    idtInitEntry(&s_idt[32], (uint32_t)isrIrq32, 0x08, C_INTERRUPT_GATE);
    idtInitEntry(&s_idt[33], (uint32_t)isrIrq33, 0x08, C_INTERRUPT_GATE);
    idtInitEntry(&s_idt[34], (uint32_t)isrIrq34, 0x08, C_INTERRUPT_GATE);
    idtInitEntry(&s_idt[35], (uint32_t)isrIrq35, 0x08, C_INTERRUPT_GATE);
    idtInitEntry(&s_idt[36], (uint32_t)isrIrq36, 0x08, C_INTERRUPT_GATE);
    idtInitEntry(&s_idt[37], (uint32_t)isrIrq37, 0x08, C_INTERRUPT_GATE);
    idtInitEntry(&s_idt[38], (uint32_t)isrIrq38, 0x08, C_INTERRUPT_GATE);
    idtInitEntry(&s_idt[39], (uint32_t)isrIrq39, 0x08, C_INTERRUPT_GATE);
    idtInitEntry(&s_idt[40], (uint32_t)isrIrq40, 0x08, C_INTERRUPT_GATE);
    idtInitEntry(&s_idt[41], (uint32_t)isrIrq41, 0x08, C_INTERRUPT_GATE);
    idtInitEntry(&s_idt[42], (uint32_t)isrIrq42, 0x08, C_INTERRUPT_GATE);
    idtInitEntry(&s_idt[43], (uint32_t)isrIrq43, 0x08, C_INTERRUPT_GATE);
    idtInitEntry(&s_idt[44], (uint32_t)isrIrq44, 0x08, C_INTERRUPT_GATE);
    idtInitEntry(&s_idt[45], (uint32_t)isrIrq45, 0x08, C_INTERRUPT_GATE);
    idtInitEntry(&s_idt[46], (uint32_t)isrIrq46, 0x08, C_INTERRUPT_GATE);
    idtInitEntry(&s_idt[47], (uint32_t)isrIrq47, 0x08, C_INTERRUPT_GATE);

    lidt(s_idt, sizeof(s_idt) - 1UL);
}

static void idtInitEntry(
    struct ts_idtEntry *p_entry,
    uint32_t p_offset,
    uint16_t p_segment,
    uint8_t p_gateType
) {
    p_entry->m_offsetLow = (uint16_t)p_offset;
    p_entry->m_segment = p_segment;
    p_entry->m_reserved = 0;
    p_entry->m_gateType = p_gateType;
    p_entry->m_zero = 0;
    p_entry->m_dpl = 0;
    p_entry->m_present = 1;
    p_entry->m_offsetHigh = (uint16_t)(p_offset >> 16U);
}
