#ifndef __INCLUDE_BOOT_LOADER_ARCH_X86_ISR_H__
#define __INCLUDE_BOOT_LOADER_ARCH_X86_ISR_H__

#include <stddef.h>
#include <stdint.h>

struct ts_isrRegisters {
    uint32_t gs, fs, es, ds;
    uint32_t esi, edi, edx, ecx, eax;
    uint32_t ebp, ebx;
    uint32_t interruptNumber, errorCode;
    uint32_t eip, cs, eflags, esp, ss;
} __attribute__((packed));

typedef void tf_isrHandler(struct ts_isrRegisters *p_registers, void *p_arg);

void isr_init(void);
void isr_setHandler(
    int p_interruptNumber,
    tf_isrHandler *p_handler,
    void *p_arg
);
void isr_setHandlerIrq(
    int p_irqNumber,
    tf_isrHandler *p_handler,
    void *p_arg
);
void isr_handler(struct ts_isrRegisters *p_registers);
void isr_exception0(void);
void isr_exception1(void);
void isr_exception2(void);
void isr_exception3(void);
void isr_exception4(void);
void isr_exception5(void);
void isr_exception6(void);
void isr_exception7(void);
void isr_exception8(void);
void isr_exception9(void);
void isr_exception10(void);
void isr_exception11(void);
void isr_exception12(void);
void isr_exception13(void);
void isr_exception14(void);
void isr_exception15(void);
void isr_exception16(void);
void isr_exception17(void);
void isr_exception18(void);
void isr_exception19(void);
void isr_exception20(void);
void isr_exception21(void);
void isr_exception22(void);
void isr_exception23(void);
void isr_exception24(void);
void isr_exception25(void);
void isr_exception26(void);
void isr_exception27(void);
void isr_exception28(void);
void isr_exception29(void);
void isr_exception30(void);
void isr_exception31(void);
void isr_irq32(void);
void isr_irq33(void);
void isr_irq34(void);
void isr_irq35(void);
void isr_irq36(void);
void isr_irq37(void);
void isr_irq38(void);
void isr_irq39(void);
void isr_irq40(void);
void isr_irq41(void);
void isr_irq42(void);
void isr_irq43(void);
void isr_irq44(void);
void isr_irq45(void);
void isr_irq46(void);
void isr_irq47(void);

#endif
