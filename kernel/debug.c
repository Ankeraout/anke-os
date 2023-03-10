
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <kernel/debug.h>

enum te_parserState {
    E_PARSERSTATE_START,
    E_PARSERSTATE_ESCAPE,
    E_PARSERSTATE_MINLEN,
    E_PARSERSTATE_MAXLEN,
    E_PARSERSTATE_ACTION_L
};

struct ts_context {
    bool a_flagPrefix;
    bool a_flagPadZero;
    bool a_flagMinimumLength;
    bool a_flagMaximumLength;
    bool a_flagLong;
    int a_minimumLength;
    int a_maximumLength;
    enum te_parserState a_state;
};

static void checkAction(
    struct ts_context *p_context,
    char p_character,
    va_list p_argList
);

static void (*s_debugWriteFunc)(void *p_parameter, char p_value) = NULL;
static void *s_debugWriteFuncParameter = NULL;

void debugInit(void (*p_writeFunc)(void *p_parameter, char p_value), void *p_parameter) {
    s_debugWriteFunc = p_writeFunc;
    s_debugWriteFuncParameter = p_parameter;
}

void debug(const char *p_format, ...) {
    va_list l_args;
    va_start(l_args, p_format);

    size_t l_index = 0;

    struct ts_context l_context = {
        .a_flagPrefix = false,
        .a_flagPadZero = false,
        .a_flagMinimumLength = false,
        .a_flagMaximumLength = false,
        .a_flagLong = false,
        .a_minimumLength = 0,
        .a_maximumLength = 0,
        .a_state = E_PARSERSTATE_START
    };

    while(p_format[l_index] != '\0') {
        char l_character = p_format[l_index++];

        switch(l_context.a_state) {
            case E_PARSERSTATE_START:
                if(l_character == '%') {
                    l_context.a_flagPrefix = false;
                    l_context.a_flagPadZero = false;
                    l_context.a_flagMinimumLength = false;
                    l_context.a_flagMaximumLength = false;
                    l_context.a_flagLong = false;
                    l_context.a_minimumLength = 0;
                    l_context.a_maximumLength = 0;
                    l_context.a_state = E_PARSERSTATE_ESCAPE;
                } else {
                    s_debugWriteFunc(s_debugWriteFuncParameter, l_character);
                }

                break;

            case E_PARSERSTATE_ESCAPE:
                if(l_character == '0') {
                    l_context.a_flagPadZero = true;
                } else if(l_character == '#') {
                    l_context.a_flagPrefix = true;
                } else if((l_character >= '1') && (l_character <= '9')) {
                    l_context.a_flagMinimumLength = true;
                    l_context.a_minimumLength = l_character - '0';
                    l_context.a_state = E_PARSERSTATE_MINLEN;
                } else if(l_character == '.') {
                    l_context.a_flagMaximumLength = true;
                    l_context.a_maximumLength = 0;
                    l_context.a_state = E_PARSERSTATE_MAXLEN;
                } else {
                    checkAction(&l_context, l_character, l_args);
                }

                break;

            case E_PARSERSTATE_MINLEN:
                if((l_character >= '0') && (l_character <= '9')) {
                    l_context.a_minimumLength *= 10;
                    l_context.a_minimumLength += l_character - '0';
                } else if(l_character == '.') {
                    l_context.a_flagMaximumLength = true;
                    l_context.a_maximumLength = 0;
                    l_context.a_state = E_PARSERSTATE_MAXLEN;
                } else {
                    checkAction(&l_context, l_character, l_args);
                }

                break;

            case E_PARSERSTATE_MAXLEN:
                if((l_character >= '0') && (l_character <= '9')) {
                    l_context.a_maximumLength *= 10;
                    l_context.a_maximumLength += l_character - '0';
                } else {
                    checkAction(&l_context, l_character, l_args);
                }

                break;

            case E_PARSERSTATE_ACTION_L:
                checkAction(&l_context, l_character, l_args);
                break;

            default:
                l_context.a_state = E_PARSERSTATE_START;
                break;
        }
    }

    va_end(l_args);
}

static void checkAction(
    struct ts_context *p_context,
    char p_character,
    va_list p_argList
) {
    if(p_character == '%') {
        s_debugWriteFunc(s_debugWriteFuncParameter, '%');
    } else if(p_character == 'c') {
        char l_character = va_arg(p_argList, int);
        s_debugWriteFunc(s_debugWriteFuncParameter, l_character);
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
            s_debugWriteFunc(s_debugWriteFuncParameter, '-');
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
            s_debugWriteFunc(s_debugWriteFuncParameter, l_buffer[l_index++]);
            l_length--;
        }

        p_context->a_state = E_PARSERSTATE_START;
    } else if(p_character == 'l') {
        p_context->a_flagLong = true;
        p_context->a_state = E_PARSERSTATE_ACTION_L;
    } else if(p_character == 'p') {
        uintptr_t l_value = va_arg(p_argList, uintptr_t);

        s_debugWriteFunc(s_debugWriteFuncParameter, '0');
        s_debugWriteFunc(s_debugWriteFuncParameter, 'x');

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
            s_debugWriteFunc(s_debugWriteFuncParameter, l_buffer[l_index++]);
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
            s_debugWriteFunc(s_debugWriteFuncParameter, l_value[l_index++]);
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
            s_debugWriteFunc(s_debugWriteFuncParameter, l_buffer[l_index++]);
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
            s_debugWriteFunc(s_debugWriteFuncParameter, '0');
            s_debugWriteFunc(s_debugWriteFuncParameter, 'x');

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
            s_debugWriteFunc(s_debugWriteFuncParameter, l_buffer[l_index++]);
            l_length--;
        }

        p_context->a_state = E_PARSERSTATE_START;
    } else {
        p_context->a_state = E_PARSERSTATE_START;
    }
}
