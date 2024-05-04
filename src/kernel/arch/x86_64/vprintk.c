#include "kernel/arch/x86_64/asm.h"
#include "kernel/printk.h"
#include "kernel/sprintf.h"

void vprintk(const char *p_format, va_list p_args) {
    char l_buffer[256];

    int l_length = vsnprintf(l_buffer, 255, p_format, p_args);

    for(int l_i = 0; l_i < l_length; l_i++) {
        outb(0xe9, l_buffer[l_i]);
    }
}
