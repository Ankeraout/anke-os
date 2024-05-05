#include "kernel/arch/x86_64/tss.h"
#include "kernel/string.h"

struct ts_tss g_tss[C_TSS_COUNT];

void tssInit(struct ts_tss *p_tss, void *p_rsp0) {
    uint64_t l_rsp0 = (uint64_t)p_rsp0;

    memset(p_tss, 0, sizeof(struct ts_tss));

    p_tss->m_iopb = sizeof(struct ts_tss);
    p_tss->m_rsp0_low = l_rsp0;
    p_tss->m_rsp0_high = l_rsp0 >> 32UL;
}
