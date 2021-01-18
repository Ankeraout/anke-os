#include "arch/i686/bioscall.h"

void video_init();
void video_puts(const char *s);
void video_putc(char c);

void video_init() {

}

void video_puts(const char *s) {
    while(*s) {
        if(*s == '\n') {
            video_putc('\r');
        }

        video_putc(*s++);
    }
}

void video_putc(char c) {
    bioscall_context_t context = {
        .ah = 0x0e,
        .al = c,
        .bh = 0,
        .bl = 0x07,
        .cx = 1
    };

    bioscall(0x10, &context);
}
