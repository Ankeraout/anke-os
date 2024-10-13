#include "boot/loader/boot.h"
#include "boot/loader/console.h"

int main(const struct ts_bootInfoStructure *p_bootInfoStructure) {
    struct ts_framebuffer l_framebuffer = {
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

    struct ts_console l_console;

    console_init(&l_console, &g_font, &l_framebuffer);
    console_write(&l_console, "Hello, world!\n", 14UL);

    while(1) {
        asm("hlt");
    }
}