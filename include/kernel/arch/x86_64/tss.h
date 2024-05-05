#ifndef __INCLUDE_KERNEL_ARCH_X86_64_TSS_H__
#define __INCLUDE_KERNEL_ARCH_X86_64_TSS_H__

#include <stdint.h>

#define C_TSS_COUNT 1

struct ts_tss {
    uint32_t m_reserved0;
    uint32_t m_rsp0_low;
    uint32_t m_rsp0_high;
    uint32_t m_rsp1_low;
    uint32_t m_rsp1_high;
    uint32_t m_rsp2_low;
    uint32_t m_rsp2_high;
    uint32_t m_reserved1;
    uint32_t m_reserved2;
    uint32_t m_ist1_low;
    uint32_t m_ist1_high;
    uint32_t m_ist2_low;
    uint32_t m_ist2_high;
    uint32_t m_ist3_low;
    uint32_t m_ist3_high;
    uint32_t m_ist4_low;
    uint32_t m_ist4_high;
    uint32_t m_ist5_low;
    uint32_t m_ist5_high;
    uint32_t m_ist6_low;
    uint32_t m_ist6_high;
    uint32_t m_ist7_low;
    uint32_t m_ist7_high;
    uint32_t m_reserved3;
    uint32_t m_reserved4;
    uint32_t m_reserved5 : 16;
    uint32_t m_iopb : 16;
} __attribute__((packed));

extern struct ts_tss g_tss[C_TSS_COUNT];

void tssInit(struct ts_tss *p_tss, void *p_rsp0);

#endif