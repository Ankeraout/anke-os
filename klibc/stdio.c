#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "klibc/stdio.h"
#include "klibc/string.h"

enum te_parserState {
    E_PARSERSTATE_START,
    E_PARSERSTATE_ESCAPE,
    E_PARSERSTATE_MINLEN,
    E_PARSERSTATE_MAXLEN,
    E_PARSERSTATE_ACTION_L
};

struct ts_kvsprintfContext {
    char *a_buffer;
    size_t a_index;
    bool a_flagPrefix;
    bool a_flagPadZero;
    bool a_flagMinimumLength;
    bool a_flagMaximumLength;
    bool a_flagLong;
    int a_minimumLength;
    int a_maximumLength;
    enum te_parserState a_state;
    size_t a_sizeLimit;
    bool a_sizeLimitEnabled;
};

static int kvsprintf_generic(
    const char *p_format,
    va_list p_args,
    struct ts_kvsprintfContext *p_context
);
static void kvsprintf_generic_checkAction(
    struct ts_kvsprintfContext *p_context,
    char p_character,
    va_list p_argList
);
static void kvsprintf_generic_out(
    struct ts_kvsprintfContext *p_context,
    char p_character
);

int ksprintf(char *restrict p_buffer, const char *restrict p_format, ...) {
    va_list l_args;
    va_start(l_args, p_format);

    int l_returnValue = kvsprintf(p_buffer, p_format, l_args);

    va_end(l_args);

    return l_returnValue;
}

int ksnprintf(
    char *restrict p_buffer,
    size_t p_size,
    const char *restrict p_format,
    ...
) {
    va_list l_args;
    va_start(l_args, p_format);

    int l_returnValue = kvsnprintf(p_buffer, p_size, p_format, l_args);

    va_end(l_args);

    return l_returnValue;
}

int kvsnprintf(
    char *p_buffer,
    size_t p_size,
    const char *p_format,
    va_list p_args
) {
    struct ts_kvsprintfContext l_context;

    kmemset(&l_context, 0, sizeof(l_context));

    l_context.a_buffer = p_buffer;
    l_context.a_sizeLimit = p_size - 1;
    l_context.a_sizeLimitEnabled = true;

    return kvsprintf_generic(p_format, p_args, &l_context);
}

int kvsprintf(char *p_buffer, const char *p_format, va_list p_args) {
    struct ts_kvsprintfContext l_context;

    kmemset(&l_context, 0, sizeof(l_context));

    l_context.a_buffer = p_buffer;

    return kvsprintf_generic(p_format, p_args, &l_context);
}

static int kvsprintf_generic(
    const char *p_format,
    va_list p_args,
    struct ts_kvsprintfContext *p_context
) {
    size_t l_index = 0;

    while(p_format[l_index] != '\0') {
        char l_character = p_format[l_index++];

        switch(p_context->a_state) {
            case E_PARSERSTATE_START:
                if(l_character == '%') {
                    p_context->a_flagPrefix = false;
                    p_context->a_flagPadZero = false;
                    p_context->a_flagMinimumLength = false;
                    p_context->a_flagMaximumLength = false;
                    p_context->a_flagLong = false;
                    p_context->a_minimumLength = 0;
                    p_context->a_maximumLength = 0;
                    p_context->a_state = E_PARSERSTATE_ESCAPE;
                } else {
                    kvsprintf_generic_out(p_context, l_character);
                }

                break;

            case E_PARSERSTATE_ESCAPE:
                if(l_character == '0') {
                    p_context->a_flagPadZero = true;
                } else if(l_character == '#') {
                    p_context->a_flagPrefix = true;
                } else if((l_character >= '1') && (l_character <= '9')) {
                    p_context->a_flagMinimumLength = true;
                    p_context->a_minimumLength = l_character - '0';
                    p_context->a_state = E_PARSERSTATE_MINLEN;
                } else if(l_character == '.') {
                    p_context->a_flagMaximumLength = true;
                    p_context->a_maximumLength = 0;
                    p_context->a_state = E_PARSERSTATE_MAXLEN;
                } else {
                    kvsprintf_generic_checkAction(p_context, l_character, p_args);
                }

                break;

            case E_PARSERSTATE_MINLEN:
                if((l_character >= '0') && (l_character <= '9')) {
                    p_context->a_minimumLength *= 10;
                    p_context->a_minimumLength += l_character - '0';
                } else if(l_character == '.') {
                    p_context->a_flagMaximumLength = true;
                    p_context->a_maximumLength = 0;
                    p_context->a_state = E_PARSERSTATE_MAXLEN;
                } else {
                    kvsprintf_generic_checkAction(p_context, l_character, p_args);
                }

                break;

            case E_PARSERSTATE_MAXLEN:
                if((l_character >= '0') && (l_character <= '9')) {
                    p_context->a_maximumLength *= 10;
                    p_context->a_maximumLength += l_character - '0';
                } else {
                    kvsprintf_generic_checkAction(p_context, l_character, p_args);
                }

                break;

            case E_PARSERSTATE_ACTION_L:
                kvsprintf_generic_checkAction(p_context, l_character, p_args);
                break;

            default:
                p_context->a_state = E_PARSERSTATE_START;
                break;
        }
    }

    p_context->a_buffer[p_context->a_index] = '\0';

    return (int)p_context->a_index;
}

static void kvsprintf_generic_checkAction(
    struct ts_kvsprintfContext *p_context,
    char p_character,
    va_list p_argList
) {
    if(p_character == '%') {
        kvsprintf_generic_out(p_context, '%');
    } else if(p_character == 'c') {
        char l_character = va_arg(p_argList, int);
        kvsprintf_generic_out(p_context, l_character);
        p_context->a_state = E_PARSERSTATE_START;
    } else if(p_character == 'd') {
        char l_buffer[32];
        int l_index = 32;
        int l_length = 0;
        long l_value = va_arg(p_argList, long);

        if(!p_context->a_flagLong) {
            l_value = (int)l_value;
        }

        bool l_negative = l_value < 0;

        if(l_negative) {
        kvsprintf_generic_out(p_context, '-');
            l_value = -l_value;
            p_context->a_minimumLength--;
        }

        do {
            l_buffer[--l_index] = '0' + (l_value % 10);
            l_value /= 10;
            l_length++;
        } while(l_value != 0);

        if(p_context->a_flagMinimumLength) {
            char l_filler;

            if(p_context->a_flagPadZero) {
                l_filler = '0';
            } else {
                l_filler = ' ';
            }

            while(l_length < p_context->a_minimumLength) {
                l_buffer[--l_index] = l_filler;
                l_length++;
            }
        }

        while(l_length > 0) {
            kvsprintf_generic_out(p_context, l_buffer[l_index++]);
            l_length--;
        }

        p_context->a_state = E_PARSERSTATE_START;
    } else if(p_character == 'l') {
        p_context->a_flagLong = true;
        p_context->a_state = E_PARSERSTATE_ACTION_L;
    } else if(p_character == 'p') {
        uintptr_t l_value = va_arg(p_argList, uintptr_t);

        kvsprintf_generic_out(p_context, '0');
        kvsprintf_generic_out(p_context, 'x');

        p_context->a_minimumLength -= 2;

        char l_buffer[32];
        int l_index = 32;
        int l_length = 0;

        do {
            l_buffer[--l_index] = "0123456789abcdef"[l_value & 0xf];
            l_value >>= 4;
            l_length++;
        } while(l_value != 0);

        if(p_context->a_flagMinimumLength) {
            char l_filler;

            if(p_context->a_flagPadZero) {
                l_filler = '0';
            } else {
                l_filler = ' ';
            }

            while(l_length < p_context->a_minimumLength) {
                l_buffer[--l_index] = l_filler;
                l_length++;
            }
        }

        while(l_length > 0) {
            kvsprintf_generic_out(p_context, l_buffer[l_index++]);
            l_length--;
        }

        p_context->a_state = E_PARSERSTATE_START;
    } else if(p_character == 's') {
        const char *l_value = va_arg(p_argList, const char *);

        int l_index = 0;

        while(
            (l_value[l_index] != '\0')
            && (
                (!p_context->a_flagMaximumLength)
                || (l_index < p_context->a_maximumLength)
            )
        ) {
            kvsprintf_generic_out(p_context, l_value[l_index++]);
        }

        p_context->a_state = E_PARSERSTATE_START;
    } else if(p_character == 'u') {
        char l_buffer[32];
        int l_index = 32;
        int l_length = 0;
        unsigned long l_value = va_arg(p_argList, unsigned long);

        if(!p_context->a_flagLong) {
            l_value = (unsigned int)l_value;
        }

        do {
            l_buffer[--l_index] = "0123456789abcdef"[l_value & 0xf];
            l_value >>= 4;
            l_length++;
        } while(l_value != 0);

        if(p_context->a_flagMinimumLength) {
            char l_filler;

            if(p_context->a_flagPadZero) {
                l_filler = '0';
            } else {
                l_filler = ' ';
            }

            while(l_length < p_context->a_minimumLength) {
                l_buffer[--l_index] = l_filler;
                l_length++;
            }
        }

        while(l_length > 0) {
            kvsprintf_generic_out(p_context, l_buffer[l_index++]);
            l_length--;
        }

        p_context->a_state = E_PARSERSTATE_START;
    } else if(p_character == 'x') {
        char l_buffer[32];
        int l_index = 32;
        int l_length = 0;
        unsigned long l_value = va_arg(p_argList, unsigned long);

        if(!p_context->a_flagLong) {
            l_value = (unsigned int)l_value;
        }

        if(p_context->a_flagPrefix) {
            kvsprintf_generic_out(p_context, '0');
            kvsprintf_generic_out(p_context, 'x');

            p_context->a_minimumLength -= 2;
        }

        do {
            l_buffer[--l_index] = "0123456789abcdef"[l_value & 0xf];
            l_value >>= 4;
            l_length++;
        } while(l_value != 0);

        if(p_context->a_flagMinimumLength) {
            char l_filler;

            if(p_context->a_flagPadZero) {
                l_filler = '0';
            } else {
                l_filler = ' ';
            }

            while(l_length < p_context->a_minimumLength) {
                l_buffer[--l_index] = l_filler;
                l_length++;
            }
        }

        while(l_length > 0) {
            kvsprintf_generic_out(p_context, l_buffer[l_index++]);
            l_length--;
        }

        p_context->a_state = E_PARSERSTATE_START;
    } else {
        p_context->a_state = E_PARSERSTATE_START;
    }
}

static void kvsprintf_generic_out(
    struct ts_kvsprintfContext *p_context,
    char p_character
) {
    if(
        (!p_context->a_sizeLimitEnabled)
        || (p_context->a_index < p_context->a_sizeLimit)
    ) {
        p_context->a_buffer[p_context->a_index++] = p_character;
    }
}
