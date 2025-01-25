#include "arch/amd64/asm.h"
#include "printk.h"
#include "sprintf.h"

void vprintk(const char *p_format, va_list p_args) {
    char l_buffer[256];

    int l_length = vsnprintf(l_buffer, 255, p_format, p_args);

    int l_i;

    if(p_format[0] >= '0' && p_format[0] <= '7') {
        l_i = 1;
    } else {
        l_i = 0;
    }

    for(; l_i < l_length; l_i++) {
        outb(0xe9, l_buffer[l_i]);
    }
}
