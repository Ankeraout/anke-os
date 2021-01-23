#include "tty.h"
#include "arch/i686/bioscall.h"

void kernel_panic(const char *msg);
static void video_puts(const char *s, uint8_t attr);
static void video_putc(char c, uint8_t attr);

void kernel_panic(const char *msg) {
    tty_write(&kernel_tty, "\n\nKernel panic: ");
    tty_write(&kernel_tty, msg);

    while(1) {
        asm("cli");
        asm("hlt");
    }
}