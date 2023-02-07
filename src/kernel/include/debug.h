#ifndef __INCLUDE_DEBUG_H__
#define __INCLUDE_DEBUG_H__

#include "dev/debugcon.h"

static inline void debugPrint(const char *p_string) {
    debugconPuts(p_string);
}

static inline void debugWrite(const void *p_buffer, size_t p_size) {
    debugconWrite(p_buffer, p_size);
}

static inline void debugPrintPointer(const void *p_ptr) {
    static const char l_hexChars[16] = "0123456789abcdef";
    uintptr_t l_ptr = (uintptr_t)p_ptr;

#ifdef __x86_64__
    const int l_digitCount = 16;
    const int l_shift = 60;
#else
    const int l_digitCount = 8;
    const int l_shift = 28;
#endif

    debugconPuts("0x");

    for(int l_digit = 0; l_digit < l_digitCount; l_digit++) {
        debugconPutc(l_hexChars[l_ptr >> l_shift]);
        l_ptr <<= 4;
    }
}

#endif
