#ifndef __KERNEL_TTY_H__
#define __KERNEL_TTY_H__

#include <stdint.h>

typedef enum {
    TTY_COLORMODE_4,
    TTY_COLORMODE_8,
    TTY_COLORMODE_24
} tty_colorMode_t;

struct tty;

typedef struct tty_api {
    int (*getTerminalWidth)(struct tty *tty);
    int (*getTerminalHeight)(struct tty *tty);
    tty_colorMode_t (*getTerminalColorMode)(struct tty *tty);
    void (*setCursorPosition)(struct tty *tty, int x, int y);
    void (*setBackgroundColor)(struct tty *tty, int color);
    void (*setForegroundColor)(struct tty *tty, int color);
    void (*write)(struct tty *tty, const char *s);
    void (*clear)(struct tty *tty);
} tty_api_t;

typedef struct tty {
    tty_api_t api;
    uint8_t reserved[256];
} tty_t;

extern tty_t kernel_tty;

int tty_getTerminalWidth(tty_t *tty);
int tty_getTerminalHeight(tty_t *tty);
tty_colorMode_t tty_getTerminalColorMode(tty_t *tty);
void tty_setCursorPosition(tty_t *tty, int x, int y);
void tty_setBackgroundColor(tty_t *tty, int color);
void tty_setForegroundColor(tty_t *tty, int color);
void tty_write(tty_t *tty, const char *s);
void tty_clear(tty_t *tty);

#endif
