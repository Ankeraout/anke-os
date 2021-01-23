#include "tty.h"

void kernel_panic(const char *msg);

void kernel_panic(const char *msg) {
    tty_write(&kernel_tty, "Kernel panic: ");
    tty_write(&kernel_tty, msg);
}