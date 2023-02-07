#include <stddef.h>

#include "arch/x86/inline.h"

void debugconPutc(int p_character) {
    outb(0xe9, p_character);
}

void debugconPuts(const char *p_string) {
    size_t l_stringIndex = 0;

    while(p_string[l_stringIndex] != '\0') {
        debugconPutc(p_string[l_stringIndex]);
        l_stringIndex++;
    }
}

void debugconWrite(const void *p_buffer, size_t p_size) {
    for(size_t l_index = 0; l_index < p_size; l_index++) {
        debugconPutc(((const uint8_t *)p_buffer)[l_index]);
    }
}
