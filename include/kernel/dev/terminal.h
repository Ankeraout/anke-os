#ifndef __INCLUDE_DEV_TERMINAL_H__
#define __INCLUDE_DEV_TERMINAL_H__

#include <kernel/dev/framebuffer.h>

struct ts_devTerminal {
    int a_x;
    int a_y;
    int a_width;
    int a_height;
    struct ts_devFramebuffer *a_framebuffer;
    struct ts_devFramebufferFont *a_font;
    t_devFramebufferColor a_foregroundColor;
    t_devFramebufferColor a_backgroundColor;
};

void terminalPutc(
    struct ts_devTerminal *p_terminal,
    char p_character
);
void terminalPuts(
    struct ts_devTerminal *p_terminal,
    const void *p_buffer
);
void terminalWrite(
    struct ts_devTerminal *p_terminal,
    const void *p_buffer,
    size_t p_size
);

#endif
