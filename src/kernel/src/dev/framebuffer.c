#include <stddef.h>
#include <stdint.h>

#include "dev/framebuffer.h"

void framebufferFillRectangle(
    struct ts_devFramebuffer *p_framebuffer,
    const struct ts_devFramebufferRectangle *p_rectangle,
    t_devFramebufferColor p_color
) {
    struct ts_devFramebufferRectangle l_rectangle;

    if(p_rectangle == NULL) {
        l_rectangle.a_x = 0;
        l_rectangle.a_y = 0;
        l_rectangle.a_width = p_framebuffer->a_width;
        l_rectangle.a_height = p_framebuffer->a_height;
    } else {
        l_rectangle = *p_rectangle;
    }

    if(l_rectangle.a_x < 0) {
        l_rectangle.a_width += l_rectangle.a_x;
        l_rectangle.a_x = 0;
    }

    if(l_rectangle.a_y < 0) {
        l_rectangle.a_height += l_rectangle.a_y;
        l_rectangle.a_y = 0;
    }

    const int l_xOverflow = l_rectangle.a_x + l_rectangle.a_width - p_framebuffer->a_width;

    if(l_xOverflow > 0) {
        l_rectangle.a_width -= l_xOverflow;
    }

    const int l_yOverflow = l_rectangle.a_y + l_rectangle.a_height - p_framebuffer->a_height;

    if(l_yOverflow > 0) {
        l_rectangle.a_height -= l_yOverflow;
    }

    if(
        (l_rectangle.a_height < 0)
        || (l_rectangle.a_width < 0)
        || ((size_t)l_rectangle.a_x >= p_framebuffer->a_width)
        || ((size_t)l_rectangle.a_y >= p_framebuffer->a_height)
    ) {
        return;
    }

    int l_y = l_rectangle.a_y;

    for(int l_row = 0; l_row < l_rectangle.a_height; l_row++) {
        int l_x = l_rectangle.a_x;

        for(int l_col = 0; l_col < l_rectangle.a_width; l_col++) {
            framebufferDrawPixel(p_framebuffer, l_x, l_y, p_color);
            l_x++;
        }

        l_y++;
    }
}

void framebufferDrawText(
    struct ts_devFramebuffer *p_framebuffer,
    const struct ts_devFramebufferFont *p_font,
    int p_x,
    int p_y,
    const char *p_str,
    t_devFramebufferColor p_foregroundColor,
    t_devFramebufferColor p_backgroundColor
) {
    int l_cx = p_x;
    size_t l_index = 0;

    while(p_str[l_index] != 0) {
        int l_y = p_y;
        uint8_t l_character = (uint8_t)p_str[l_index];
        int l_fontOffset = l_character * p_font->a_height;
        const uint8_t *l_fontBuffer = (const uint8_t *)p_font->a_buffer;

        for(int l_row = 0; l_row < p_font->a_height; l_row++) {
            uint8_t l_rowData = l_fontBuffer[l_fontOffset++];
            int l_x = l_cx;

            for(int l_col = 0; l_col < 8; l_col++) {
                t_devFramebufferColor l_color;

                if((l_rowData & 0x80) != 0) {
                    l_color = p_foregroundColor;
                } else {
                    l_color = p_backgroundColor;
                }

                framebufferDrawPixel(p_framebuffer, l_x, l_y, l_color);

                l_rowData <<= 1;
                l_x++;
            }

            l_x = l_cx;
            l_y++;
        }

        l_cx += 8;
        l_y = p_y;
        l_index++;
    }
}

void framebufferDrawPixel(
    struct ts_devFramebuffer *p_framebuffer,
    int p_x,
    int p_y,
    t_devFramebufferColor p_color
) {
    if(
        (p_x < 0)
        || (p_y < 0)
        || ((size_t)p_x >= p_framebuffer->a_width)
        || ((size_t)p_y >= p_framebuffer->a_height)
    ) {
        return;
    }

    const size_t l_index = (p_framebuffer->a_pitch >> 2) * p_y + p_x;
    uint32_t *l_buffer = (uint32_t *)p_framebuffer->a_buffer;
    uint32_t l_oldPixel = l_buffer[l_index];
    uint8_t l_oldRed = l_oldPixel >> 16;
    uint8_t l_oldGreen = l_oldPixel >> 8;
    uint8_t l_oldBlue = l_oldPixel;
    uint8_t l_alpha = p_color >> 24;
    uint8_t l_oldAlpha = 255 - l_alpha;
    uint8_t l_newRed = p_color >> 16;
    uint8_t l_newGreen = p_color >> 8;
    uint8_t l_newBlue = p_color;
    uint8_t l_red = (l_oldAlpha * l_oldRed + l_alpha * l_newRed) / 255;
    uint8_t l_green = (l_oldAlpha * l_oldGreen + l_alpha * l_newGreen) / 255;
    uint8_t l_blue = (l_oldAlpha * l_oldBlue + l_alpha * l_newBlue) / 255;
    uint32_t l_color = (l_red << 16) | (l_green << 8) | l_blue;

    l_buffer[l_index] = l_color;
}

void framebufferScrollUp(
    struct ts_devFramebuffer *p_framebuffer,
    int p_n,
    t_devFramebufferColor p_color
) {
    if(p_n < 0) {
        return;
    }

    if((size_t)p_n > p_framebuffer->a_height) {
        p_n = p_framebuffer->a_height;
    }

    size_t l_nextLine = (p_framebuffer->a_pitch >> 2) - p_framebuffer->a_width;
    size_t l_sourceIndex = (p_framebuffer->a_pitch >> 2) * p_n;
    size_t l_destinationIndex = 0;
    size_t l_rows = p_framebuffer->a_height - p_n;

    for(size_t l_row = 0; l_row < l_rows; l_row++) {
        for(size_t l_col = 0; l_col < p_framebuffer->a_width; l_col++) {
            ((uint32_t *)p_framebuffer->a_buffer)[l_destinationIndex++] = ((uint32_t *)p_framebuffer->a_buffer)[l_sourceIndex++];
        }

        l_sourceIndex += l_nextLine;
        l_destinationIndex += l_nextLine;
    }

    for(size_t l_row = 0; l_row < (size_t)p_n; l_row++) {
        for(size_t l_col = 0; l_col < p_framebuffer->a_width; l_col++) {
            ((uint32_t *)p_framebuffer->a_buffer)[l_destinationIndex++] = p_color;
        }
    }
}
