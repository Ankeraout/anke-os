#ifndef __INCLUDE_BIOSCALL_H__
#define __INCLUDE_BIOSCALL_H__

#include <stdint.h>

struct ts_bioscallRegisters {
    uint32_t m_eax;
    uint32_t m_ebx;
    uint32_t m_ecx;
    uint32_t m_edx;
    uint32_t m_ebp;
    uint32_t m_esi;
    uint32_t m_edi;
    uint32_t m_eflags;
    uint32_t m_ds;
    uint32_t m_es;
} __attribute__((packed));

void bioscall(uint8_t p_vector, struct ts_bioscallRegisters *p_registers);

#endif
