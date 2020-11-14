#ifndef __KERNEL_ARCH_I686_IO_HPP__
#define __KERNEL_ARCH_I686_IO_HPP__

#include <stdint.h>

namespace kernel {
    static inline void cli() {
        asm("cli");
    }

    static inline void sti() {
        asm("sti");
    }

    static inline void hlt() {
        asm("hlt");
    }

    static inline void outb(uint16_t port, uint8_t value) {
        asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
    }

    static inline void outw(uint16_t port, uint16_t value) {
        asm volatile("outw %0, %1" : : "a"(value), "Nd"(port));
    }

    static inline void outl(uint16_t port, uint32_t value) {
        asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
    }

    static inline uint8_t inb(uint16_t port) {
        uint8_t returnValue;

        asm volatile("inb %1, %0": "=a"(returnValue) : "Nd"(port));
        
        return returnValue;
    }

    static inline uint16_t inw(uint16_t port) {
        uint16_t returnValue;

        asm volatile("inw %1, %0": "=a"(returnValue) : "Nd"(port));
        
        return returnValue;
    }

    static inline uint32_t inl(uint16_t port) {
        uint32_t returnValue;

        asm volatile("inl %1, %0": "=a"(returnValue) : "Nd"(port));
        
        return returnValue;
    }

    static inline void io_wait() {
        asm volatile("outb %%al, $0x80" : : "a"(0));
    }

    static inline void lidt(void *base, uint16_t size) {
        struct {
            uint16_t length;
            void *base;
        } __attribute__((packed)) idtr = {size, base};

        asm("lidt %0" : : "m"(idtr));
    }

    static inline uint64_t rdtsc() {
        uint64_t returnValue;

        asm volatile("rdtsc" : "=A"(returnValue));

        return returnValue;
    }

    static inline void invlpg(const void* ptr) {
        asm volatile("invlpg (%0)" : : "b"(ptr) : "memory");
    }

    static inline void cpuid(int code, uint32_t* a, uint32_t* d) {
        asm volatile("cpuid" : "=a"(*a), "=d"(*d) : "0"(code) : "ebx", "ecx");
    }

    static inline uint64_t rdmsr(uint32_t msr_id) {
        uint64_t msr_value;

        asm volatile("rdmsr" : "=A"(msr_value) : "c"(msr_id));

        return msr_value;
    }

    static inline void wrmsr(uint32_t msr_id, uint64_t msr_value) {
        asm volatile("wrmsr" : : "c"(msr_id), "A"(msr_value));
    }
}

#endif
