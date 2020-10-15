#include "tty/tty.h"

extern tty_t kernel_tty;
extern void halt();

void kernel_panic(const char *message) {
    kernel_tty.attr = 0x0c;
    tty_puts(&kernel_tty, "Kernel panic!\n");
    tty_puts(&kernel_tty, message);

    halt();
}
