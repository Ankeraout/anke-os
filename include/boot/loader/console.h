#ifndef __INCLUDE_BOOT_LOADER_CONSOLE_H__
#define __INCLUDE_BOOT_LOADER_CONSOLE_H__

#include <stddef.h>

#include "boot/loader/font.h"
#include "boot/loader/framebuffer.h"
#include "boot/loader/types.h"

struct ts_console {
    struct ts_framebuffer *m_framebuffer;
    const struct ts_font *m_font;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_x;
    unsigned int m_y;
    uint32_t m_foregroundColor;
    uint32_t m_backgroundColor;
};

int console_init(
    struct ts_console *p_console,
    const struct ts_font *p_font,
    struct ts_framebuffer *p_framebuffer
);
ssize_t console_write(
    struct ts_console *p_console,
    const void *p_buffer,
    size_t p_size
);

#endif
