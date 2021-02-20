#include <stddef.h>

#include "tty.h"
#include "arch/i686/bioscall.h"

void tty_bios_init(tty_t *tty);

static int tty_bios_getTerminalWidth(tty_t *tty);
static int tty_bios_getTerminalHeight(tty_t *tty);
static tty_colorMode_t tty_bios_getTerminalColorMode(tty_t *tty);
static void tty_bios_setBackgroundColor(struct tty *tty, int color);
static void tty_bios_setForegroundColor(struct tty *tty, int color);
static void tty_bios_write(struct tty *tty, const char *s);
static void tty_bios_getCursorPosition(tty_t *tty, int *x, int *y);
static void tty_bios_setCursorPosition(tty_t *tty, int x, int y);
static void tty_bios_clear(struct tty *tty);

static tty_api_t tty_bios = {
    .getTerminalWidth = tty_bios_getTerminalWidth,
    .getTerminalHeight = tty_bios_getTerminalHeight,
    .getTerminalColorMode = tty_bios_getTerminalColorMode,
    .setBackgroundColor = tty_bios_setBackgroundColor,
    .setForegroundColor = tty_bios_setForegroundColor,
    .getCursorPosition = tty_bios_getCursorPosition,
    .setCursorPosition = tty_bios_setCursorPosition,
    .write = tty_bios_write,
    .clear = tty_bios_clear
};

void tty_bios_init(tty_t *tty) {
    tty->api = tty_bios;
}

static int tty_bios_getTerminalWidth(tty_t *tty) {
    // Remove compilation warning about unused parameter
    (void)tty;

    return 80;
}

static int tty_bios_getTerminalHeight(tty_t *tty) {
    // Remove compilation warning about unused parameter
    (void)tty;

    return 50;
}

static tty_colorMode_t tty_bios_getTerminalColorMode(tty_t *tty) {
    // Remove compilation warning about unused parameter
    (void)tty;

    return TTY_COLORMODE_4;
}

static void tty_bios_setBackgroundColor(struct tty *tty, int color) {
    // Remove compilation warning about unused parameter
    (void)tty;
    (void)color;
}

static void tty_bios_setForegroundColor(struct tty *tty, int color) {
    // Remove compilation warning about unused parameter
    (void)tty;
    (void)color;
}

static void tty_bios_writeChar(char c) {
    bioscall_context_t context = {
        .ah = 0x0e,
        .al = c,
        .bh = 0,
        .bl = 0x07
    };

    bioscall(0x10, &context);
}

static void tty_bios_write(struct tty *tty, const char *s) {
    (void)tty;

    for(size_t i = 0; s[i] != '\0'; i++) {
        if(s[i] == '\n') {
            tty_bios_writeChar('\r');
            tty_bios_writeChar('\n');
        } else {
            tty_bios_writeChar(s[i]);
        }
    }
}

static void tty_bios_getCursorPosition(tty_t *tty, int *x, int *y) {
    bioscall_context_t context = {
        .ah = 0x03,
        .bh = 0
    };

    bioscall(0x10, &context);

    *x = context.dl;
    *y = context.dh;
}

static void tty_bios_setCursorPosition(tty_t *tty, int x, int y) {
    // Remove compilation warning about unused parameter
    (void)tty;
    
    bioscall_context_t context = {
        .ah = 0x03,
        .bh = 0,
        .dl = x,
        .dh = y
    };

    bioscall(0x10, &context);
}

static void tty_bios_clear(struct tty *tty) {
    // Remove compilation warning about unused parameter
    (void)tty;
}
