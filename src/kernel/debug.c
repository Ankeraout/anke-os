#include "tty/tty.h"

extern tty_t kernel_tty;

void kernel_debug(const char *message) {
    tty_puts(&kernel_tty, message);
}
