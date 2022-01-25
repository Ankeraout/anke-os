#ifndef __INC_ARCH_X86_ASM_H__
#define __INC_ARCH_X86_ASM_H__

#include <stdint.h>

static inline void cli(void) {
    asm volatile("cli");
}

static inline void sti(void) {
    asm volatile("sti");
}

static inline void hlt(void) {
    asm volatile("hlt");
}

static inline uint8_t inb(uint16_t p_port) {
    uint8_t l_value;

    asm volatile("inb %1, %0" : "=a"(l_value) : "Nd"(p_port));

    return l_value;
}

static inline uint16_t inw(uint16_t p_port) {
    uint16_t l_value;

    asm volatile("inw %1, %0" : "=a"(l_value) : "Nd"(p_port));

    return l_value;
}

static inline uint32_t inl(uint16_t p_port) {
    uint32_t l_value;

    asm volatile("inl %1, %0" : "=a"(l_value) : "Nd"(p_port));

    return l_value;
}

static inline void lgdt(void *p_base, uint16_t p_size) {
    struct {
        uint16_t size;
        uint32_t base;
    } __attribute__((packed)) l_gdtr = {
        .base = (uint32_t)p_base,
        .size = p_size
    };

    asm("lgdt %0" :: "m"(l_gdtr));
}

static inline void lidt(void *p_base, uint16_t p_size) {
    struct {
        uint16_t size;
        uint32_t base;
    } __attribute__((packed)) l_idtr = {
        .base = (uint32_t)p_base,
        .size = p_size
    };

    asm("lidt %0" :: "m"(l_idtr));
}

static inline void outb(uint16_t p_port, uint8_t p_value) {
    asm volatile("outb %0, %1" :: "a"(p_value), "Nd"(p_port));
}

static inline void outw(uint16_t p_port, uint16_t p_value) {
    asm volatile("outw %0, %1" :: "a"(p_value), "Nd"(p_port));
}

static inline void outl(uint16_t p_port, uint32_t p_value) {
    asm volatile("outl %0, %1" :: "a"(p_value), "Nd"(p_port));
}

static inline void ioWait(void) {
    outb(0x80, 0x00);
}

static inline uint64_t rdtsc(void) {
    uint64_t l_value;

    asm volatile("rdtsc" : "=A"(l_value));

    return l_value;
}

static inline void invlpg(void *p_address) {
    asm volatile("invlpg (%0)" :: "b"(p_address) : "memory");
}

#endif // __INC_ARCH_X86_ASM_H__
