#include "arch/amd64/tss.h"

struct ts_tss g_tss[C_TSS_COUNT];

void tss_initTss(struct ts_tss *p_tss, uint64_t p_rsp0) {
    p_tss->m_rsp[0].m_0_31 = p_rsp0;
    p_tss->m_rsp[0].m_32_63 = p_rsp0 >> 32;
}
