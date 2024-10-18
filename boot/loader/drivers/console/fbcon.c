#include <stdbool.h>

#include "boot/loader/drivers/console/fbcon.h"

static void drawCharacter(
    struct ts_fbcon *p_console,
    unsigned int p_x,
    unsigned int p_y,
    unsigned char p_char
);

int fbcon_init(
    struct ts_fbcon *p_fbcon,
    struct ts_framebuffer *p_framebuffer
) {
    p_fbcon->m_framebuffer = p_framebuffer;
    p_fbcon->m_width = p_framebuffer->m_width / g_font.m_characterWidth;
    p_fbcon->m_height = g_font.m_characterHeight / g_font.m_characterHeight;
    p_fbcon->m_x = 0U;
    p_fbcon->m_y = 0U;
    p_fbcon->m_foregroundColor =
        framebuffer_createColor(p_framebuffer, 255U, 255U, 255U);
    p_fbcon->m_backgroundColor =
        framebuffer_createColor(p_framebuffer, 0U, 0U, 0U);

    return 0;
}

ssize_t fbcon_write(
    struct ts_fbcon *p_fbcon,
    const void *p_buffer,
    size_t p_size
) {
    const uint8_t *l_str = (const uint8_t *)p_buffer;
    size_t l_index = 0UL;

    while(l_index < p_size) {
        const uint8_t l_character = l_str[l_index++];

        if(l_character == '\0') {
            // Do nothing
        } else if(l_character == '\t') {
            p_fbcon->m_x += 4U - (p_fbcon->m_x % 4U);
        } else if(l_character == '\n') {
            p_fbcon->m_x = 0U;
            p_fbcon->m_y++;
        } else if(l_character == '\r') {
            p_fbcon->m_x = 0U;
        } else {
            drawCharacter(
                p_fbcon,
                p_fbcon->m_x * g_font.m_characterWidth,
                p_fbcon->m_y * g_font.m_characterHeight,
                l_character
            );

            p_fbcon->m_x++;
        }

        if(p_fbcon->m_x >= p_fbcon->m_width) {
            p_fbcon->m_x = 0U;
            p_fbcon->m_y++;
        }

        if(p_fbcon->m_y >= p_fbcon->m_height) {
            p_fbcon->m_y = 0U;
        }
    }

    return (ssize_t)p_size;
}

static void drawCharacter(
    struct ts_fbcon *p_fbcon,
    unsigned int p_x,
    unsigned int p_y,
    unsigned char p_char
) {
    uint8_t *p_fbBuffer = (uint8_t *)p_fbcon->m_framebuffer->m_buffer;

    for(unsigned int l_y = 0U; l_y < 16U; l_y++) {
        uint8_t l_row = g_font.m_fontData[p_char * 16U + l_y];
        uint32_t *l_fbRow =
            (uint32_t *)&p_fbBuffer[
                p_fbcon->m_framebuffer->m_pitch * (p_y + l_y)
            ];

        for(unsigned int l_x = 0U; l_x < 8U; l_x++) {
            uint32_t l_color;

            if((l_row & 0x80U) != 0U) {
                l_color = p_fbcon->m_foregroundColor;
            } else {
                l_color = p_fbcon->m_backgroundColor;
            }

            l_fbRow[p_x + l_x] = l_color;

            l_row <<= 1U;
        }
    }
}
