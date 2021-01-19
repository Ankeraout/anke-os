#include "arch/i686/bioscall.h"

void kernel_panic(const char *msg);
static void video_puts(const char *s, uint8_t attr);
static void video_putc(char c, uint8_t attr);

void kernel_panic(const char *msg) {
    video_puts("\n\nKernel panic:", 0x4f);
    video_puts(msg, 0x4f);

    while(1) {
        asm("cli");
        asm("hlt");
    }
}

static void video_puts(const char *s, uint8_t attr) {
    while(*s) {
        if(*s == '\n') {
            video_putc('\r', attr);
        }

        video_putc(*s++, attr);
    }
}

static void video_putc(char c, uint8_t attr) {
    bioscall_context_t context = {
        .ah = 0x09,
        .al = c,
        .bh = 0,
        .bl = attr,
        .cx = 1
    };

    bioscall(0x10, &context);
}