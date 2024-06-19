#ifndef __INCLUDE_ARCH_X86_ISR_H__
#define __INCLUDE_ARCH_X86_ISR_H__

#include <stdint.h>

struct ts_isrRegisters {
    uint32_t m_gs, m_fs, m_es, m_ds;
    uint32_t m_ebp, m_edi, m_esi;
    uint32_t m_edx, m_ecx, m_ebx, m_eax;
    uint32_t m_interruptNumber, m_errorCode;
    uint32_t m_eip, m_cs, m_eflags, m_esp, m_ss;
} __attribute__((packed));

typedef void tf_interruptHandler(void *p_arg);

void isrHandler(struct ts_isrRegisters *p_registers);
void isrInit(void);
void isrAdd(int p_interrupt, tf_interruptHandler *p_handler, void *p_arg);
void isrRemove(int p_interrupt, tf_interruptHandler *p_handler, void *p_arg);

#endif
