#ifndef __TTY_H__
#define __TTY_H__

#include <stdint.h>

typedef struct {
    int w;
    int h;
    int x;
    int y;
    void *buf;
    uint8_t attr;
} tty_t;

void tty_cls(tty_t *tty);
void tty_putc(tty_t *tty, const char c);
void tty_puts(tty_t *tty, const char *s);

#endif
