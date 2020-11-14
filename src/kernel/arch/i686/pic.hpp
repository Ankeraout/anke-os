#ifndef __KERNEL_ARCH_I686_PIC_HPP__
#define __KERNEL_ARCH_I686_PIC_HPP__

#include <stdint.h>

namespace kernel {
    const uint8_t PIC1 = 0x20;
    const uint8_t PIC2 = 0xa0;
    const uint8_t PIC_EOI = 0x20;

    void pic_init();
    void pic_enableIRQ(int irqNumber);
    void pic_disableIRQ(int irqNumber);
    void pic_enableIRQs();
    void pic_disableIRQs();
}

#endif
