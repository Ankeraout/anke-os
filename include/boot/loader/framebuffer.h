#ifndef __INCLUDE_BOOT_LOADER_FRAMEBUFFER_H__
#define __INCLUDE_BOOT_LOADER_FRAMEBUFFER_H__

#include <stdint.h>

#include "boot/loader/boot.h"

struct ts_framebuffer {
    void *m_buffer;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_pitch;
    uint32_t m_bpp;
    uint32_t m_redBits;
    uint32_t m_redShift;
    uint32_t m_greenBits;
    uint32_t m_greenShift;
    uint32_t m_blueBits;
    uint32_t m_blueShift;
};

void framebuffer_init(const struct ts_bootInfoStructure *p_bootInfoStructure);
uint32_t framebuffer_createColor(
    struct ts_framebuffer *p_framebuffer,
    uint8_t p_red,
    uint8_t p_green,
    uint8_t p_blue
);
struct ts_framebuffer *framebuffer_get(void);

#endif
