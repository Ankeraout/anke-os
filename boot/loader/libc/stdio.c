#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include "arch/x86/asm.h"
#include "stdio.h"
#include "string.h"

struct ts_cursorPosition {
    unsigned int m_x;
    unsigned int m_y;
};
enum te_parserState {
    E_PARSERSTATE_START,
    E_PARSERSTATE_ESCAPE,
    E_PARSERSTATE_MINLEN,
    E_PARSERSTATE_MAXLEN,
    E_PARSERSTATE_ACTION_L
};

struct ts_vsprintfContext {
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
static void get_cursor_position(struct ts_cursorPosition *p_cursorPosition);
static void set_cursor_position(
    const struct ts_cursorPosition *p_cursorPosition
);

static struct ts_cursorPosition s_cursorPosition;

void stdio_init(void) {
    get_cursor_position(&s_cursorPosition);
}

int putchar(int p_character) {
    uint8_t *l_vram = (uint8_t *)0xb8000;

    if(p_character == '\t') {
        s_cursorPosition.m_x += 4U - (s_cursorPosition.m_x % 4U);
    } else if(p_character == '\n') {
        s_cursorPosition.m_x = 0U;
        s_cursorPosition.m_y++;
    } else if(p_character == '\r') {
        s_cursorPosition.m_x = 0U;
    } else {
        unsigned int l_index =
            (s_cursorPosition.m_y * 80U + s_cursorPosition.m_x) * 2U;
        l_vram[l_index] = p_character;
        l_vram[l_index + 1] = 0x07U;
        s_cursorPosition.m_x++;
    }

    if(s_cursorPosition.m_x >= 80U) {
        s_cursorPosition.m_x = 0U;
        s_cursorPosition.m_y++;
    }

    if(s_cursorPosition.m_y >= 25U) {
        s_cursorPosition.m_y = 0U;
    }

    set_cursor_position(&s_cursorPosition);

    return p_character;
}

int puts(const char *p_string) {
    int l_returnValue = 0;

    while(*p_string != '\0') {
        putchar(*p_string++);
        l_returnValue++;
    }

    return l_returnValue;
}

int printf(const char *restrict p_format, ...) {
    va_list l_args;
    va_start(l_args, p_format);

    char l_buffer[C_PRINTF_BUFFER_LENGTH];

    int l_returnValue = vsnprintf(
        l_buffer,
        C_PRINTF_BUFFER_LENGTH - 1,
        p_format,
        l_args
    );

    puts(l_buffer);

    va_end(l_args);

    return l_returnValue;
}

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

    l_context.a_buffer = p_buffer;
    l_context.a_sizeLimit = p_size - 1;
    l_context.a_sizeLimitEnabled = true;

    return vsprintf_generic(p_format, p_args, &l_context);
}

int vsprintf(char *p_buffer, const char *p_format, va_list p_args) {
    struct ts_vsprintfContext l_context;

    memset(&l_context, 0, sizeof(l_context));

    l_context.a_buffer = p_buffer;

    return vsprintf_generic(p_format, p_args, &l_context);
}

static void get_cursor_position(struct ts_cursorPosition *p_cursorPosition) {
    outb(0x3d4U, 0x0fU);
    uint16_t l_cursorPosition = inb(0x3d5U);
    outb(0x3d4U, 0x0eU);
    l_cursorPosition |= inb(0x3d5U) << 8U;
    
    p_cursorPosition->m_x = l_cursorPosition % 80U;
    p_cursorPosition->m_y = l_cursorPosition / 80U;
}

static void set_cursor_position(
    const struct ts_cursorPosition *p_cursorPosition
) {
    uint16_t l_cursorPosition =
        p_cursorPosition->m_y * 80U + p_cursorPosition->m_x;

    outb(0x3d4U, 0x0fU);
    outb(0x3d5U, l_cursorPosition);
    outb(0x3d4U, 0x0eU);
    outb(0x3d5U, l_cursorPosition >> 8U);
}

static int vsprintf_generic(
    const char *p_format,
    va_list p_args,
    struct ts_vsprintfContext *p_context
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
                    vsprintf_generic_out(p_context, l_character);
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
                    vsprintf_generic_checkAction(p_context, l_character, p_args);
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
                    vsprintf_generic_checkAction(p_context, l_character, p_args);
                }

                break;

            case E_PARSERSTATE_MAXLEN:
                if((l_character >= '0') && (l_character <= '9')) {
                    p_context->a_maximumLength *= 10;
                    p_context->a_maximumLength += l_character - '0';
                } else {
                    vsprintf_generic_checkAction(p_context, l_character, p_args);
                }

                break;

            case E_PARSERSTATE_ACTION_L:
                vsprintf_generic_checkAction(p_context, l_character, p_args);
                break;

            default:
                p_context->a_state = E_PARSERSTATE_START;
                break;
        }
    }

    p_context->a_buffer[p_context->a_index] = '\0';

    return (int)p_context->a_index;
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
            vsprintf_generic_out(p_context, '-');
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
            vsprintf_generic_out(p_context, l_buffer[l_index++]);
            l_length--;
        }

        p_context->a_state = E_PARSERSTATE_START;
    } else if(p_character == 'l') {
        p_context->a_flagLong = true;
        p_context->a_state = E_PARSERSTATE_ACTION_L;
    } else if(p_character == 'p') {
        uintptr_t l_value = va_arg(p_argList, uintptr_t);

        vsprintf_generic_out(p_context, '0');
        vsprintf_generic_out(p_context, 'x');

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
            vsprintf_generic_out(p_context, l_buffer[l_index++]);
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
            vsprintf_generic_out(p_context, l_value[l_index++]);
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
            vsprintf_generic_out(p_context, l_buffer[l_index++]);
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
            vsprintf_generic_out(p_context, '0');
            vsprintf_generic_out(p_context, 'x');

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
            vsprintf_generic_out(p_context, l_buffer[l_index++]);
            l_length--;
        }

        p_context->a_state = E_PARSERSTATE_START;
    } else {
        p_context->a_state = E_PARSERSTATE_START;
    }
}

static void vsprintf_generic_out(
    struct ts_vsprintfContext *p_context,
    char p_character
) {
    if(
        (!p_context->a_sizeLimitEnabled)
        || (p_context->a_index < p_context->a_sizeLimit)
    ) {
        p_context->a_buffer[p_context->a_index++] = p_character;
    }
}
