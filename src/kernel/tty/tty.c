#include "libk/libk.h"
#include "tty/tty.h"

void tty_cls(tty_t *tty) {
    memset(tty->buf, 0, tty->w * tty->h * 2);

    tty->x = 0;
    tty->y = 0;
    tty->attr = 0x07;
}

void tty_scrollup(tty_t *tty, int n) {
    memcpy(tty->buf, tty->buf + 2 * tty->w * n, tty->w * (tty->h - n) * 2);
    memset(tty->buf + 2 * tty->w * (tty->h - n), 0, tty->w * n * 2);

    tty->y -= n;
}

void tty_putc(tty_t *tty, const char c) {
    if(c == 9) {
        tty->x += 4 - (tty->x % 4);
    } else if(c == '\n') {
        tty->x = 0;
        tty->y++;
    } else if(c == '\r') {
        tty->x = 0;
    } else {
        ((uint8_t *)tty->buf)[(tty->y * tty->w + tty->x) * 2] = c;
        ((uint8_t *)tty->buf)[(tty->y * tty->w + tty->x) * 2 + 1] = tty->attr;
        tty->x++;
    }

    if(tty->x >= tty->w) {
        tty->x = 0;
        tty->y++;
    }

    if(tty->y >= tty->h) {
        tty_scrollup(tty, tty->y - tty->h + 1);
    }
}

void tty_puts(tty_t *tty, const char *s) {
    while(*s) {
        tty_putc(tty, *s++);
    }
}
