#include "boot/loader/boot.h"
#include "boot/loader/drivers/cmos.h"
#include "boot/loader/drivers/console/console.h"
#include "boot/loader/drivers/console/fbcon.h"
#include "boot/loader/stdio.h"

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

    console_init();
    fbcon_init(&l_framebuffer);
    printf("Hello, world!\n");

    while(1) {
        asm("hlt");
    }
}


