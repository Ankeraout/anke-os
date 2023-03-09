#ifndef __INCLUDE_MODULES_FRAMEBUFFER_H__
#define __INCLUDE_MODULES_FRAMEBUFFER_H__

#include <stdint.h>

#include <kernel/fonts/fonts.h>

typedef uint32_t t_framebufferColor;

enum {
    E_IOCTL_FRAMEBUFFER_CREATE = 1,
    E_IOCTL_FRAMEBUFFER_GET_WIDTH,
    E_IOCTL_FRAMEBUFFER_GET_HEIGHT,
    E_IOCTL_FRAMEBUFFER_FILL,
    E_IOCTL_FRAMEBUFFER_DRAW_CHARACTER,
    E_IOCTL_FRAMEBUFFER_SCROLL_UP,
    E_IOCTL_FRAMEBUFFER_SET_PIXEL
};

struct ts_framebufferRequestCreate {
    void *a_buffer;
    size_t a_width;
    size_t a_height;
    size_t a_pitch;
};

struct ts_framebufferRequestFill {
    t_framebufferColor a_color;
};

struct ts_framebufferRequestDrawCharacter {
    int a_x;
    int a_y;
    t_framebufferColor a_backgroundColor;
    t_framebufferColor a_foregroundColor;
    char a_character;
    struct ts_consoleFont *a_font;
};

struct ts_framebufferRequestScrollUp {
    int a_rows;
    t_framebufferColor a_color;
};

struct ts_framebufferRequestSetPixel {
    size_t a_x;
    size_t a_y;
    t_framebufferColor a_color;
};

#endif
