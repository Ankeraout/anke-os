#ifndef __INCLUDE_ARCH_AMD64_ASM_H__
#define __INCLUDE_ARCH_AMD64_ASM_H__

#include <stdint.h>

static inline void asm_outb(uint16_t p_port, uint8_t p_value) {
    asm volatile("outb %0, %1" :: "a"(p_value), "Nd"(p_port));
}

static inline void asm_outw(uint16_t p_port, uint16_t p_value) {
    asm volatile("outw %0, %1" :: "a"(p_value), "Nd"(p_port));
}

static inline void asm_outl(uint16_t p_port, uint32_t p_value) {
    asm volatile("outl %0, %1" :: "a"(p_value), "Nd"(p_port));
}

static inline uint64_t asm_readCr3(void) {
    uint64_t l_returnValue;

    asm volatile("movq %%cr3, %0": "=a"(l_returnValue));

    return l_returnValue;
}

static inline void asm_writeCr3(uint64_t p_value) {
    asm volatile("movq %0, %%cr3" :: "a"(p_value));
}

static inline uint8_t asm_inb(uint16_t p_port) {
    uint8_t l_returnValue;

    asm volatile("inb %1, %0" : "=a"(l_returnValue) : "Nd"(p_port));

    return l_returnValue;
}

static inline uint16_t asm_inw(uint16_t p_port) {
    uint16_t l_returnValue;

    asm volatile("inw %1, %0" : "=a"(l_returnValue) : "Nd"(p_port));

    return l_returnValue;
}

static inline uint32_t asm_inl(uint16_t p_port) {
    uint32_t l_returnValue;

    asm volatile("inl %1, %0" : "=a"(l_returnValue) : "Nd"(p_port));

    return l_returnValue;
}

static inline void asm_iowait(void) {
    asm_outb(0x80, 0x00);
}

static inline void asm_lidt(void *p_base, uint16_t p_size) {
    struct {
        uint16_t m_size;
        void *m_base;
    } __attribute__((packed)) l_idtr = {p_size, p_base};

    asm("lidt %0" :: "m"(l_idtr));
}

static inline void asm_lgdt(void *p_base, uint16_t p_size) {
    struct {
        uint16_t m_size;
        void *m_base;
    } __attribute__((packed)) l_gdtr = {p_size, p_base};

    asm("lgdt %0" :: "m"(l_gdtr));
}

static inline void asm_cli(void) {
    asm("cli");
}

static inline void asm_sti(void) {
    asm("sti");
}

static inline void asm_hlt(void) {
    asm("hlt");
}

static inline void asm_ltr(uint16_t p_selector) {
    asm("ltr %0" :: "r"(p_selector));
}

#endif
