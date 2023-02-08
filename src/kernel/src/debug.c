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

void debugWrite(const void *p_buffer, size_t p_size) {
    for(size_t l_index = 0; l_index < p_size; l_index++) {
        s_debugWriteFunc(s_debugWriteFuncParameter, ((const uint8_t *)p_buffer)[l_index]);
    }
}

void debugPrintPointer(const void *p_ptr) {
    static const char l_hexChars[16] = "0123456789abcdef";
    uintptr_t l_ptr = (uintptr_t)p_ptr;

#ifdef __x86_64__
    const int l_digitCount = 16;
    const int l_shift = 60;
#else
    const int l_digitCount = 8;
    const int l_shift = 28;
#endif

    debugWrite("0x", 2);

    for(int l_digit = 0; l_digit < l_digitCount; l_digit++) {
        debugWrite(&l_hexChars[l_ptr >> l_shift], 1);
        l_ptr <<= 4;
    }
}
