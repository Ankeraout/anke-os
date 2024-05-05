#include <stdbool.h>
#include <stdint.h>

#include "kernel/sprintf.h"
#include "kernel/string.h"

enum te_parserState {
    E_PARSERSTATE_START,
    E_PARSERSTATE_ESCAPE,
    E_PARSERSTATE_MINLEN,
    E_PARSERSTATE_MAXLEN,
    E_PARSERSTATE_ACTION_L
};

struct ts_vsprintfContext {
    char *m_buffer;
    size_t m_index;
    bool m_flagPrefix;
    bool m_flagPadZero;
    bool m_flagMinimumLength;
    bool m_flagMaximumLength;
    bool m_flagLong;
    int m_minimumLength;
    int m_maximumLength;
    enum te_parserState m_state;
    size_t m_sizeLimit;
    bool m_sizeLimitEnabled;
};

static int vsprintf_generic(
    const char *p_format,
    va_list p_args,
    struct ts_vsprintfContext *p_context
);
static void vsprintf_generic_checkAction(
    struct ts_vsprintfContext *p_context,
    char p_character,
    va_list p_argList
);
static void vsprintf_generic_out(
    struct ts_vsprintfContext *p_context,
    char p_character
);

int sprintf(char *restrict p_buffer, const char *restrict p_format, ...) {
    va_list l_args;
    va_start(l_args, p_format);

    int l_returnValue = vsprintf(p_buffer, p_format, l_args);

    va_end(l_args);

    return l_returnValue;
}

int snprintf(
    char *restrict p_buffer,
    size_t p_size,
    const char *restrict p_format,
    ...
) {
    va_list l_args;
    va_start(l_args, p_format);

    int l_returnValue = vsnprintf(p_buffer, p_size, p_format, l_args);

    va_end(l_args);

    return l_returnValue;
}

int vsnprintf(
    char *p_buffer,
    size_t p_size,
    const char *p_format,
    va_list p_args
) {
    struct ts_vsprintfContext l_context;

    memset(&l_context, 0, sizeof(l_context));

    l_context.m_buffer = p_buffer;
    l_context.m_sizeLimit = p_size - 1;
    l_context.m_sizeLimitEnabled = true;

    return vsprintf_generic(p_format, p_args, &l_context);
}

int vsprintf(char *p_buffer, const char *p_format, va_list p_args) {
    struct ts_vsprintfContext l_context;

    memset(&l_context, 0, sizeof(l_context));

    l_context.m_buffer = p_buffer;

    return vsprintf_generic(p_format, p_args, &l_context);
}

static int vsprintf_generic(
    const char *p_format,
    va_list p_args,
    struct ts_vsprintfContext *p_context
) {
    size_t l_index = 0;

    while(p_format[l_index] != '\0') {
        char l_character = p_format[l_index++];

        switch(p_context->m_state) {
            case E_PARSERSTATE_START:
                if(l_character == '%') {
                    p_context->m_flagPrefix = false;
                    p_context->m_flagPadZero = false;
                    p_context->m_flagMinimumLength = false;
                    p_context->m_flagMaximumLength = false;
                    p_context->m_flagLong = false;
                    p_context->m_minimumLength = 0;
                    p_context->m_maximumLength = 0;
                    p_context->m_state = E_PARSERSTATE_ESCAPE;
                } else {
                    vsprintf_generic_out(p_context, l_character);
                }

                break;

            case E_PARSERSTATE_ESCAPE:
                if(l_character == '0') {
                    p_context->m_flagPadZero = true;
                } else if(l_character == '#') {
                    p_context->m_flagPrefix = true;
                } else if((l_character >= '1') && (l_character <= '9')) {
                    p_context->m_flagMinimumLength = true;
                    p_context->m_minimumLength = l_character - '0';
                    p_context->m_state = E_PARSERSTATE_MINLEN;
                } else if(l_character == '.') {
                    p_context->m_flagMaximumLength = true;
                    p_context->m_maximumLength = 0;
                    p_context->m_state = E_PARSERSTATE_MAXLEN;
                } else {
                    vsprintf_generic_checkAction(p_context, l_character, p_args);
                }

                break;

            case E_PARSERSTATE_MINLEN:
                if((l_character >= '0') && (l_character <= '9')) {
                    p_context->m_minimumLength *= 10;
                    p_context->m_minimumLength += l_character - '0';
                } else if(l_character == '.') {
                    p_context->m_flagMaximumLength = true;
                    p_context->m_maximumLength = 0;
                    p_context->m_state = E_PARSERSTATE_MAXLEN;
                } else {
                    vsprintf_generic_checkAction(p_context, l_character, p_args);
                }

                break;

            case E_PARSERSTATE_MAXLEN:
                if((l_character >= '0') && (l_character <= '9')) {
                    p_context->m_maximumLength *= 10;
                    p_context->m_maximumLength += l_character - '0';
                } else {
                    vsprintf_generic_checkAction(p_context, l_character, p_args);
                }

                break;

            case E_PARSERSTATE_ACTION_L:
                vsprintf_generic_checkAction(p_context, l_character, p_args);
                break;

            default:
                p_context->m_state = E_PARSERSTATE_START;
                break;
        }
    }

    p_context->m_buffer[p_context->m_index] = '\0';

    return (int)p_context->m_index;
}

static void vsprintf_generic_checkAction(
    struct ts_vsprintfContext *p_context,
    char p_character,
    va_list p_argList
) {
    if(p_character == '%') {
        vsprintf_generic_out(p_context, '%');
    } else if(p_character == 'c') {
        char l_character = va_arg(p_argList, int);
        vsprintf_generic_out(p_context, l_character);
        p_context->m_state = E_PARSERSTATE_START;
    } else if(p_character == 'd') {
        char l_buffer[32];
        int l_index = 32;
        int l_length = 0;
        long l_value = va_arg(p_argList, long);

        if(!p_context->m_flagLong) {
            l_value = (int)l_value;
        }

        bool l_negative = l_value < 0;

        if(l_negative) {
            vsprintf_generic_out(p_context, '-');
            l_value = -l_value;
            p_context->m_minimumLength--;
        }

        do {
            l_buffer[--l_index] = '0' + (l_value % 10);
            l_value /= 10;
            l_length++;
        } while(l_value != 0);

        if(p_context->m_flagMinimumLength) {
            char l_filler;

            if(p_context->m_flagPadZero) {
                l_filler = '0';
            } else {
                l_filler = ' ';
            }

            while(l_length < p_context->m_minimumLength) {
                l_buffer[--l_index] = l_filler;
                l_length++;
            }
        }

        while(l_length > 0) {
            vsprintf_generic_out(p_context, l_buffer[l_index++]);
            l_length--;
        }

        p_context->m_state = E_PARSERSTATE_START;
    } else if(p_character == 'l') {
        p_context->m_flagLong = true;
        p_context->m_state = E_PARSERSTATE_ACTION_L;
    } else if(p_character == 'p') {
        uintptr_t l_value = va_arg(p_argList, uintptr_t);

        vsprintf_generic_out(p_context, '0');
        vsprintf_generic_out(p_context, 'x');

        p_context->m_minimumLength -= 2;

        char l_buffer[32];
        int l_index = 32;
        int l_length = 0;

        do {
            l_buffer[--l_index] = "0123456789abcdef"[l_value & 0xf];
            l_value >>= 4;
            l_length++;
        } while(l_value != 0);

        if(p_context->m_flagMinimumLength) {
            char l_filler;

            if(p_context->m_flagPadZero) {
                l_filler = '0';
            } else {
                l_filler = ' ';
            }

            while(l_length < p_context->m_minimumLength) {
                l_buffer[--l_index] = l_filler;
                l_length++;
            }
        }

        while(l_length > 0) {
            vsprintf_generic_out(p_context, l_buffer[l_index++]);
            l_length--;
        }

        p_context->m_state = E_PARSERSTATE_START;
    } else if(p_character == 's') {
        const char *l_value = va_arg(p_argList, const char *);

        int l_index = 0;

        while(
            (l_value[l_index] != '\0')
            && (
                (!p_context->m_flagMaximumLength)
                || (l_index < p_context->m_maximumLength)
            )
        ) {
            vsprintf_generic_out(p_context, l_value[l_index++]);
        }

        p_context->m_state = E_PARSERSTATE_START;
    } else if(p_character == 'u') {
        char l_buffer[32];
        int l_index = 32;
        int l_length = 0;
        unsigned long l_value = va_arg(p_argList, unsigned long);

        if(!p_context->m_flagLong) {
            l_value = (unsigned int)l_value;
        }

        do {
            l_buffer[--l_index] = '0' + (l_value % 10);
            l_value /= 10;
            l_length++;
        } while(l_value != 0);

        if(p_context->m_flagMinimumLength) {
            char l_filler;

            if(p_context->m_flagPadZero) {
                l_filler = '0';
            } else {
                l_filler = ' ';
            }

            while(l_length < p_context->m_minimumLength) {
                l_buffer[--l_index] = l_filler;
                l_length++;
            }
        }

        while(l_length > 0) {
            vsprintf_generic_out(p_context, l_buffer[l_index++]);
            l_length--;
        }

        p_context->m_state = E_PARSERSTATE_START;
    } else if(p_character == 'x') {
        char l_buffer[32];
        int l_index = 32;
        int l_length = 0;
        unsigned long l_value = va_arg(p_argList, unsigned long);

        if(!p_context->m_flagLong) {
            l_value = (unsigned int)l_value;
        }

        if(p_context->m_flagPrefix) {
            vsprintf_generic_out(p_context, '0');
            vsprintf_generic_out(p_context, 'x');

            p_context->m_minimumLength -= 2;
        }

        do {
            l_buffer[--l_index] = "0123456789abcdef"[l_value & 0xf];
            l_value >>= 4;
            l_length++;
        } while(l_value != 0);

        if(p_context->m_flagMinimumLength) {
            char l_filler;

            if(p_context->m_flagPadZero) {
                l_filler = '0';
            } else {
                l_filler = ' ';
            }

            while(l_length < p_context->m_minimumLength) {
                l_buffer[--l_index] = l_filler;
                l_length++;
            }
        }

        while(l_length > 0) {
            vsprintf_generic_out(p_context, l_buffer[l_index++]);
            l_length--;
        }

        p_context->m_state = E_PARSERSTATE_START;
    } else {
        p_context->m_state = E_PARSERSTATE_START;
    }
}

static void vsprintf_generic_out(
    struct ts_vsprintfContext *p_context,
    char p_character
) {
    if(
        (!p_context->m_sizeLimitEnabled)
        || (p_context->m_index < p_context->m_sizeLimit)
    ) {
        p_context->m_buffer[p_context->m_index++] = p_character;
    }
}
