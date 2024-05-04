#ifndef __INCLUDE_KERNEL_ARCH_X86_64_ASM_H__
#define __INCLUDE_KERNEL_ARCH_X86_64_ASM_H__

#include <stdint.h>

static inline void outb(uint16_t p_port, uint8_t p_value) {
    asm volatile("outb %0, %1" :: "a"(p_value), "Nd"(p_port));
}

static inline void outw(uint16_t p_port, uint16_t p_value) {
    asm volatile("outw %0, %1" :: "a"(p_value), "Nd"(p_port));
}

static inline void outl(uint16_t p_port, uint32_t p_value) {
    asm volatile("outl %0, %1" :: "a"(p_value), "Nd"(p_port));
}

static inline uint64_t readCr3(void) {
    uint64_t l_returnValue;

    asm volatile("movq %%cr3, %0": "=a"(l_returnValue));

    return l_returnValue;
}

static inline void writeCr3(uint64_t p_value) {
    asm volatile("movq %0, %%cr3" :: "a"(p_value));
}

static inline uint8_t inb(uint16_t p_port) {
    uint8_t l_returnValue;

    asm volatile("inb %1, %0" : "=a"(l_returnValue) : "Nd"(p_port));

    return l_returnValue;
}

static inline uint16_t inw(uint16_t p_port) {
    uint16_t l_returnValue;

    asm volatile("inw %1, %0" : "=a"(l_returnValue) : "Nd"(p_port));

    return l_returnValue;
}

static inline uint32_t inl(uint16_t p_port) {
    uint32_t l_returnValue;

    asm volatile("inl %1, %0" : "=a"(l_returnValue) : "Nd"(p_port));

    return l_returnValue;
}

static inline void iowait(void) {
    outb(0x80, 0x00);
}

static inline void lidt(void *p_base, uint16_t p_size) {
    struct {
        uint16_t a_size;
        void *a_base;
    } __attribute__((packed)) l_idtr = {p_size, p_base};

    asm("lidt %0" :: "m"(l_idtr));
}

static inline void lgdt(void *p_base, uint16_t p_size) {
    struct {
        uint16_t a_size;
        void *a_base;
    } __attribute__((packed)) l_gdtr = {p_size, p_base};

    asm("lgdt %0" :: "m"(l_gdtr));
}

static inline void cli(void) {
    asm("cli");
}

static inline void sti(void) {
    asm("sti");
}

static inline void hlt(void) {
    asm("hlt");
}

#endif