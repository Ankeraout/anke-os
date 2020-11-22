#ifndef __KERNEL_ARCH_I686_ISR_HPP__
#define __KERNEL_ARCH_I686_ISR_HPP__

extern "C" {
    extern void isr_handler_0();
    extern void isr_handler_1();
    extern void isr_handler_2();
    extern void isr_handler_3();
    extern void isr_handler_4();
    extern void isr_handler_5();
    extern void isr_handler_6();
    extern void isr_handler_7();
    extern void isr_handler_8();
    extern void isr_handler_9();
    extern void isr_handler_10();
    extern void isr_handler_11();
    extern void isr_handler_12();
    extern void isr_handler_13();
    extern void isr_handler_14();
    extern void isr_handler_15();
    extern void isr_handler_exception();
    extern void isr_registerIRQHandler(int irq, void handler(void *), void *arg);
}

#endif
