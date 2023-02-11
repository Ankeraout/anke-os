#ifndef __INCLUDE_DEV_FRAMEBUFFER_H__
#define __INCLUDE_DEV_FRAMEBUFFER_H__

#include <stddef.h>
#include <stdint.h>

struct ts_devFramebuffer {
    void *a_buffer;
    size_t a_width;
    size_t a_height;
    size_t a_pitch;
};

struct ts_devFramebufferRectangle {
    int a_x;
    int a_y;
    int a_width;
    int a_height;
};

struct ts_devFramebufferFont {
    int a_height;
    void *a_buffer;
};

typedef uint32_t t_devFramebufferColor;

void framebufferFillRectangle(
    struct ts_devFramebuffer *p_framebuffer,
    const struct ts_devFramebufferRectangle *p_rectangle,
    t_devFramebufferColor p_color
);
void framebufferDrawText(
    struct ts_devFramebuffer *p_framebuffer,
    const struct ts_devFramebufferFont *p_font,
    int p_x,
    int p_y,
    const char *p_str,
    t_devFramebufferColor p_foregroundColor,
    t_devFramebufferColor p_backgroundColor
);
void framebufferDrawPixel(
    struct ts_devFramebuffer *p_framebuffer,
    int p_x,
    int p_y,
    t_devFramebufferColor p_color
);
void framebufferScrollUp(
    struct ts_devFramebuffer *p_framebuffer,
    int p_n,
    t_devFramebufferColor p_color
);

#endif
