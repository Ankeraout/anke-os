#include "tty.h"

int tty_getTerminalWidth(tty_t *tty);
int tty_getTerminalHeight(tty_t *tty);
tty_colorMode_t tty_getTerminalColorMode(tty_t *tty);
void tty_setCursorPosition(tty_t *tty, int x, int y);
void tty_setBackgroundColor(tty_t *tty, int color);
void tty_setForegroundColor(tty_t *tty, int color);
void tty_write(tty_t *tty, const char *s);

int tty_getTerminalWidth(tty_t *tty) {
    return tty->api.getTerminalWidth(tty);
}

int tty_getTerminalHeight(tty_t *tty) {
    return tty->api.getTerminalHeight(tty);
}

tty_colorMode_t tty_getTerminalColorMode(tty_t *tty) {
    return tty->api.getTerminalColorMode(tty);
}

void tty_setCursorPosition(tty_t *tty, int x, int y) {
    return tty->api.setCursorPosition(tty, x, y);
}

void tty_setBackgroundColor(tty_t *tty, int color) {
    return tty->api.setBackgroundColor(tty, color);
}

void tty_setForegroundColor(tty_t *tty, int color) {
    return tty->api.setForegroundColor(tty, color);
}

void tty_write(tty_t *tty, const char *s) {
    return tty->api.write(tty, s);
}
