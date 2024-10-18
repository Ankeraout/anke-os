#ifndef __INCLUDE_BOOT_LOADER_DRIVERS_CONSOLE_FBCON_H__
#define __INCLUDE_BOOT_LOADER_DRIVERS_CONSOLE_FBCON_H__

#include <stddef.h>

#include "boot/loader/font.h"
#include "boot/loader/framebuffer.h"
#include "boot/loader/types.h"

struct ts_fbcon {
    struct ts_framebuffer *m_framebuffer;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_x;
    unsigned int m_y;
    uint32_t m_foregroundColor;
    uint32_t m_backgroundColor;
};

int fbcon_init(
    struct ts_fbcon *p_fbcon,
    struct ts_framebuffer *p_framebuffer
);
ssize_t fbcon_write(
    struct ts_fbcon *p_fbcon,
    const void *p_buffer,
    size_t p_size
);

#endif
