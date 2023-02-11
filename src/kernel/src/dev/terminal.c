#include "dev/framebuffer.h"
#include "dev/terminal.h"

void terminalWrite(
    struct ts_devTerminal *p_terminal,
    const void *p_buffer,
    size_t p_size
) {
    const uint8_t *l_buffer = (const uint8_t *)p_buffer;
    
    for(size_t l_index = 0; l_index < p_size; l_index++) {
        uint8_t l_character = l_buffer[l_index];

        if(l_character == '\r') {
            p_terminal->a_x = 0;
        } else if(l_character == '\n') {
            p_terminal->a_x = 0;
            p_terminal->a_y++;
        } else if(l_character == '\t') {
            p_terminal->a_x += 4 - (p_terminal->a_x % 4);
        } else if(l_character != 0) {
            uint8_t l_text[] = {l_character, 0};

            framebufferDrawText(
                p_terminal->a_framebuffer,
                p_terminal->a_font,
                p_terminal->a_x * 8,
                p_terminal->a_y * p_terminal->a_font->a_height,
                (const char *)l_text,
                p_terminal->a_foregroundColor,
                p_terminal->a_backgroundColor
            );

            p_terminal->a_x++;
        }

        if(p_terminal->a_x >= p_terminal->a_width) {
            p_terminal->a_x = 0;
            p_terminal->a_y++;
        }

        if(p_terminal->a_y >= p_terminal->a_height) {
            int p_n = (p_terminal->a_y - p_terminal->a_height + 1);

            framebufferScrollUp(
                p_terminal->a_framebuffer,
                p_n * p_terminal->a_font->a_height,
                p_terminal->a_backgroundColor
            );

            p_terminal->a_y -= p_n;
        }
    }
}
