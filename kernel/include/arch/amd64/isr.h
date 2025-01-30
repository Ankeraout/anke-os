#ifndef __INCLUDE_ARCH_AMD64_ISR_H__
#define __INCLUDE_ARCH_AMD64_ISR_H__

#include <stddef.h>
#include <stdint.h>

struct ts_isrRegisters {
    uint64_t m_gs, m_fs, m_es, m_ds;
    uint64_t m_r11, m_r10, m_r9, m_r8;
    uint64_t m_rsi, m_rdi, m_rdx, m_rcx, m_rax;
    uint64_t m_r15, m_r14, m_r13, m_r12, m_rbp, m_rbx;
    uint64_t m_interruptNumber, m_errorCode;
    uint64_t m_rip, m_cs, m_rflags, m_rsp, m_ss;
} __attribute__((packed));

typedef void tf_isrHandler(struct ts_isrRegisters *p_registers, void *p_arg);

void isrInit(void);
void isrSetHandler(int p_interruptNumber, tf_isrHandler *p_handler, void *p_arg);
void isrHandler(struct ts_isrRegisters *p_registers);
void isrException0(void);
void isrException1(void);
void isrException2(void);
void isrException3(void);
void isrException4(void);
void isrException5(void);
void isrException6(void);
void isrException7(void);
void isrException8(void);
void isrException9(void);
void isrException10(void);
void isrException11(void);
void isrException12(void);
void isrException13(void);
void isrException14(void);
void isrException15(void);
void isrException16(void);
void isrException17(void);
void isrException18(void);
void isrException19(void);
void isrException20(void);
void isrException21(void);
void isrException22(void);
void isrException23(void);
void isrException24(void);
void isrException25(void);
void isrException26(void);
void isrException27(void);
void isrException28(void);
void isrException29(void);
void isrException30(void);
void isrException31(void);
void isrIrq32(void);
void isrIrq33(void);
void isrIrq34(void);
void isrIrq35(void);
void isrIrq36(void);
void isrIrq37(void);
void isrIrq38(void);
void isrIrq39(void);
void isrIrq40(void);
void isrIrq41(void);
void isrIrq42(void);
void isrIrq43(void);
void isrIrq44(void);
void isrIrq45(void);
void isrIrq46(void);
void isrIrq47(void);

#endif
