#ifndef __INCLUDE_ARCH_AMD64_TSS_H__
#define __INCLUDE_ARCH_AMD64_TSS_H__

#include <stdint.h>

#include "arch/amd64/asm.h"

#define C_TSS_COUNT CONFIG_MAX_CORE_COUNT

struct ts_tss_value {
    uint32_t m_0_31;
    uint32_t m_32_63;
};

struct ts_tss {
    uint32_t m_reserved;
    struct ts_tss_value m_rsp[3];
    struct ts_tss_value m_reserved_2;
    struct ts_tss_value m_ist[7];
    struct ts_tss_value m_reserved_3;
    uint32_t m_reserved_4 : 16;
    uint32_t m_ioMapBaseAddress : 16;
};

extern struct ts_tss g_tss[];

void tss_initTss(struct ts_tss *p_tss, uint64_t p_rsp0);

#endif
