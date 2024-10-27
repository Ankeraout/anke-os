#include <stddef.h>

#include "boot/loader/boot.h"
#include "boot/loader/framebuffer.h"

static struct ts_framebuffer s_framebuffer = {
    .m_buffer = NULL
};

void framebuffer_init(const struct ts_bootInfoStructure *p_bootInfoStructure) {
    const struct ts_framebuffer l_framebuffer = {
        .m_width = p_bootInfoStructure->m_framebufferWidth,
        .m_height = p_bootInfoStructure->m_framebufferHeight,
        .m_pitch = p_bootInfoStructure->m_framebufferPitch,
        .m_bpp = p_bootInfoStructure->m_framebufferBpp,
        .m_buffer = (void *)p_bootInfoStructure->m_framebufferAddress,
        .m_redBits = p_bootInfoStructure->m_framebufferRedBits,
        .m_redShift = p_bootInfoStructure->m_framebufferRedShift,
        .m_greenBits = p_bootInfoStructure->m_framebufferGreenBits,
        .m_greenShift = p_bootInfoStructure->m_framebufferGreenShift,
        .m_blueBits = p_bootInfoStructure->m_framebufferBlueBits,
        .m_blueShift = p_bootInfoStructure->m_framebufferBlueShift
    };

    s_framebuffer = l_framebuffer;
}

uint32_t framebuffer_createColor(
    struct ts_framebuffer *p_framebuffer,
    uint8_t p_red,
    uint8_t p_green,
    uint8_t p_blue
) {
    return (p_red << p_framebuffer->m_redShift)
        | (p_green << p_framebuffer->m_greenShift)
        | (p_blue << p_framebuffer->m_blueShift);
}

struct ts_framebuffer *framebuffer_get(void) {
    if(s_framebuffer.m_buffer == NULL) {
        return NULL;
    } else {
        return &s_framebuffer;
    }
}
