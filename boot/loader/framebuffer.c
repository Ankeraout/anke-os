#include "boot/loader/framebuffer.h"

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
