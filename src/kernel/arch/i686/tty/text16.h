#ifndef __KERNEL_ARCH_I686_TTY_TEXT16_H__
#define __KERNEL_ARCH_I686_TTY_TEXT16_H__

#include <stdint.h>

#include "tty.h"

void tty_text16_init(tty_t *tty, void *buffer, int terminalWidth, int terminalHeight, void (*setCursorPosition)(tty_t *tty, int x, int y));

#endif
