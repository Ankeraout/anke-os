#ifndef __INCLUDE_BOOT_LOADER_FONT_H__
#define __INCLUDE_BOOT_LOADER_FONT_H__

#include <stdint.h>

struct ts_font {
    unsigned int m_characterWidth;
    unsigned int m_characterHeight;
    uint8_t m_fontData[];
};

extern const struct ts_font g_font;

#endif
