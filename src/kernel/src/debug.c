#include <stddef.h>
#include <stdint.h>

#include "debug.h"

static t_debugWriteFunc s_debugWriteFunc = NULL;
static void *s_debugWriteFuncParameter = NULL;

void debugInit(t_debugWriteFunc p_writeFunc, void *p_parameter) {
    s_debugWriteFunc = p_writeFunc;
    s_debugWriteFuncParameter = p_parameter;
}

void debugPrint(const char *p_string) {
    size_t l_index = 0;

    while(p_string[l_index] != '\0') {
        s_debugWriteFunc(s_debugWriteFuncParameter, p_string[l_index]);
        l_index++;
    }
}

void debugPrintHex8(uint8_t p_value) {
    static const char l_hexChars[16] = "0123456789abcdef";

    for(int l_digit = 0; l_digit < 2; l_digit++) {
        s_debugWriteFunc(s_debugWriteFuncParameter, l_hexChars[p_value >> 4]);
        p_value <<= 4;
    }
}

void debugPrintHex16(uint16_t p_value) {
    static const char l_hexChars[16] = "0123456789abcdef";

    for(int l_digit = 0; l_digit < 4; l_digit++) {
        s_debugWriteFunc(s_debugWriteFuncParameter, l_hexChars[p_value >> 12]);
        p_value <<= 4;
    }
}

void debugPrintHex32(uint32_t p_value) {
    static const char l_hexChars[16] = "0123456789abcdef";

    for(int l_digit = 0; l_digit < 8; l_digit++) {
        s_debugWriteFunc(s_debugWriteFuncParameter, l_hexChars[p_value >> 28]);
        p_value <<= 4;
    }
}

void debugPrintHex64(uint64_t p_value) {
    static const char l_hexChars[16] = "0123456789abcdef";

    for(int l_digit = 0; l_digit < 16; l_digit++) {
        s_debugWriteFunc(s_debugWriteFuncParameter, l_hexChars[p_value >> 60]);
        p_value <<= 4;
    }
}

void debugWrite(const void *p_buffer, size_t p_size) {
    for(size_t l_index = 0; l_index < p_size; l_index++) {
        s_debugWriteFunc(s_debugWriteFuncParameter, ((const uint8_t *)p_buffer)[l_index]);
    }
}

void debugPrintPointer(const void *p_ptr) {
    uintptr_t l_ptr = (uintptr_t)p_ptr;

#ifdef __x86_64__
    debugPrintHex64(l_ptr);
#else
    debugPrintHex32(l_ptr);
#endif
}
