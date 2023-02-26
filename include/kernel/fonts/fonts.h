#ifndef __INCLUDE_FONTS_FONTS_H__
#define __INCLUDE_FONTS_FONTS_H__

struct ts_consoleFont {
    int a_width;
    uint8_t a_data[];
};

extern struct ts_consoleFont g_font8;
extern struct ts_consoleFont g_font16;

#endif
