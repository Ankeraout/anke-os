#ifndef __INCLUDE_FONTS_FONTS_H__
#define __INCLUDE_FONTS_FONTS_H__

#include <stdint.h>

struct ts_consoleFont {
    int a_height;
    uint8_t a_data[];
};

extern struct ts_consoleFont g_font8;
extern struct ts_consoleFont g_font16;

#endif
