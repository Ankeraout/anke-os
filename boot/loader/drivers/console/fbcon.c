#include <stdbool.h>

#include "boot/loader/drivers/console/console.h"
#include "boot/loader/drivers/console/fbcon.h"

static void drawCharacter(
    struct ts_fbcon *p_console,
    unsigned int p_x,
    unsigned int p_y,
    unsigned char p_char
);
static ssize_t fbcon_write(
    struct ts_console *p_console,
    const void *p_buffer,
    size_t p_size
);

static struct ts_fbcon s_fbcon;

int fbcon_init(struct ts_framebuffer *p_framebuffer) {
    s_fbcon.m_framebuffer = p_framebuffer;
    s_fbcon.m_width = p_framebuffer->m_width / g_font.m_characterWidth;
    s_fbcon.m_height = p_framebuffer->m_height / g_font.m_characterHeight;
    s_fbcon.m_x = 0U;
    s_fbcon.m_y = 0U;
    s_fbcon.m_foregroundColor =
        framebuffer_createColor(p_framebuffer, 255U, 255U, 255U);
    s_fbcon.m_backgroundColor =
        framebuffer_createColor(p_framebuffer, 0U, 0U, 0U);

    // Register console
    struct ts_console *l_console = console_alloc();

    if(l_console == NULL) {
        // Failed to allocate console
        return -1;
    }

    l_console->driverData = &s_fbcon;
    l_console->write = fbcon_write;

    // Register console
    console_register(l_console);

    // TODO: what if the console could not be registered?

    return 0;
}

static ssize_t fbcon_write(
    struct ts_console *p_console,
    const void *p_buffer,
    size_t p_size
) {
    struct ts_fbcon *l_fbcon = (struct ts_fbcon *)p_console->driverData;
    const uint8_t *l_str = (const uint8_t *)p_buffer;
    size_t l_index = 0UL;

    while(l_index < p_size) {
        const uint8_t l_character = l_str[l_index++];

        if(l_character == '\0') {
            // Do nothing
        } else if(l_character == '\t') {
            l_fbcon->m_x += 4U - (l_fbcon->m_x % 4U);
        } else if(l_character == '\n') {
            l_fbcon->m_x = 0U;
            l_fbcon->m_y++;
        } else if(l_character == '\r') {
            l_fbcon->m_x = 0U;
        } else {
            drawCharacter(
                l_fbcon,
                l_fbcon->m_x * g_font.m_characterWidth,
                l_fbcon->m_y * g_font.m_characterHeight,
                l_character
            );

            l_fbcon->m_x++;
        }

        if(l_fbcon->m_x >= l_fbcon->m_width) {
            l_fbcon->m_x = 0U;
            l_fbcon->m_y++;
        }

        if(l_fbcon->m_y >= l_fbcon->m_height) {
            l_fbcon->m_y = 0U;
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
