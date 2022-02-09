// =============================================================================
// File inclusion
// =============================================================================
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "klibc/klibc.h"

// =============================================================================
// Private variables declaration
// =============================================================================
/**
 * @brief This structure defines the context for a xprintf call.
 */
struct ts_printfContext {
    const char *format;
    va_list *argumentList;
    char *buffer;
    size_t bufferSize;
    char bufferBase[128];
};

// =============================================================================
// Private functions declaration
// =============================================================================
/**
 * @brief Initializes the printf context.
 *
 * @param[out] p_context The context to initialize.
 * @param[in] p_format The format of the formatted string.
 * @param[in] p_argumentList The argument list.
 */
static void initContext(
    struct ts_printfContext *p_context,
    const char *p_format,
    va_list *p_argumentList
);

/**
 * @brief Gets the next character in the formatted string.
 *
 * @param[in,out] p_context The printf context.
 *
 * @returns The next character in the formatted string.
 */
static char getNextCharacter(struct ts_printfContext *p_context);

/**
 * @brief Transforms an integer into a string.
 *
 * @param[out] p_buffer The output buffer.
 * @param[in] p_integer The integer to convert.
 * @param[in] p_base The base of the number.
 * @param[in] p_putPlusSign Whether or not to put the "plus" sign if the number
 *                          is positive.
 * @param[in] p_putSpaceSign Whether or not to put a space if the number is
 *                           positive.
 * @param[in] p_paddingAmount The total amount of characters with padding.
 * @param[in] p_justify Whether or not to left-justify the result.
 * @param[in] p_padWithZeros Whether or not the number shall be padded with
 *                           zeros instead of spaces.
 *
 * @returns A pointer to the given buffer.
 */
static char *integerToString(
    char *p_buffer,
    int64_t p_integer,
    int p_base,
    bool p_putPlusSign,
    bool p_putSpaceSign,
    int p_paddingAmount,
    bool p_justify,
    bool p_padWithZeros
);

// =============================================================================
// Public functions definition
// =============================================================================
int kprintf(const char *p_format, ...) {
    va_list l_argumentList;
    va_start(l_argumentList, p_format);

    int l_returnValue = kvprintf(p_format, l_argumentList);

    va_end(l_argumentList);

    return l_returnValue;
}

int ksprintf(char *p_buffer, const char *p_format, ...) {
    va_list l_argumentList;
    va_start(l_argumentList, p_format);

    int l_returnValue = kvsprintf(p_buffer, p_format, l_argumentList);

    va_end(l_argumentList);

    return l_returnValue;
}

int ksnprintf(char *p_buffer, size_t p_bufferSize, const char *p_format, ...) {
    va_list l_argumentList;
    va_start(l_argumentList, p_format);

    int l_returnValue = kvsnprintf(p_buffer, p_bufferSize, p_format, l_argumentList);

    va_end(l_argumentList);

    return l_returnValue;
}

int kvsnprintf(
    char *p_buffer,
    size_t p_bufferSize,
    const char *p_format,
    va_list p_argumentList
) {
    struct ts_printfContext l_context;

    initContext(&l_context, p_format, &p_argumentList);

    size_t l_bufferIndex = 0;
    char l_nextCharacter = getNextCharacter(&l_context);

    while((l_nextCharacter != '\0') && (l_bufferIndex < p_bufferSize)) {
        p_buffer[l_bufferIndex++] = l_nextCharacter;
    }

    p_buffer[l_bufferIndex] = '\0';

    return l_bufferIndex;
}

int kvsprintf(char *p_buffer, const char *p_format, va_list p_argumentList) {
    struct ts_printfContext l_context;

    initContext(&l_context, p_format, &p_argumentList);

    int l_bufferIndex = 0;
    char l_nextCharacter = getNextCharacter(&l_context);

    while(l_nextCharacter != '\0') {
        p_buffer[l_bufferIndex++] = l_nextCharacter;
    }

    p_buffer[l_bufferIndex] = '\0';

    return l_bufferIndex;
}

int kvprintf(const char *p_format, va_list p_argumentList) {
    struct ts_printfContext l_context;

    initContext(&l_context, p_format, &p_argumentList);

    int l_bufferIndex = 0;
    char l_nextCharacter = getNextCharacter(&l_context);

    while(l_nextCharacter != '\0') {
        kputchar(l_nextCharacter);
        l_bufferIndex++;
    }

    return l_bufferIndex;
}

// =============================================================================
// Private functions definition
// =============================================================================
static void initContext(
    struct ts_printfContext *p_context,
    const char *p_format,
    va_list *p_argumentList
) {
    kmemset(p_context, 0, sizeof(*p_context));

    p_context->format = p_format;
    p_context->argumentList = p_argumentList;
}

static char getNextCharacter(struct ts_printfContext *p_context) {
    if(p_context->bufferSize > 0) {

    }
}

static char *integerToString(
    char *p_buffer,
    int64_t p_integer,
    int p_base,
    bool p_putPlusSign,
    bool p_putSpaceSign,
    int p_paddingAmount,
    bool p_justify,
    bool p_padWithZeros
) {
    char l_digits[16];
    int64_t l_integer = p_integer;

    for(int l_digitIndex = 0; l_digitIndex < 16; l_digitIndex++) {
        if(l_digitIndex < 10) {
            l_digits[l_digitIndex] = '0' + l_digitIndex;
        } else if(p_base == 16) {
            l_digits[l_digitIndex] = 'a' + l_digitIndex - 10;
        } else {
            l_digits[l_digitIndex] = 'A' + l_digitIndex - 10;
        }
    }

    char *l_buffer = p_buffer;

    if(p_base == 10) {
        if(p_integer < 0) {
            *l_buffer++ = '-';
            l_integer = -l_integer;
        } else if(p_putPlusSign) {
            *l_buffer++ = '+';
        } else if(p_putSpaceSign) {
            *l_buffer++ = ' ';
        }
    }

    int64_t l_shift = p_integer;

    do {
        l_buffer++;
        l_shift = l_shift / p_base;
    } while(l_shift > 0);

    *l_buffer = '\0';

    do {
        l_buffer--;
        *l_buffer = l_digits[l_integer % p_base];
        l_integer = l_integer / p_base;
    } while(l_integer > 0);

    int l_padding = p_paddingAmount - (int)kstrlen(p_buffer);

    if(l_padding < 0) {
        l_padding = 0;
    }

    if(p_justify) {
        while((l_padding--) > 0) {
            if(p_padWithZeros) {
                p_buffer[kstrlen(p_buffer)] = '0';
            } else {
                p_buffer[kstrlen(p_buffer)] = ' ';
            }
        }
    } else {
        char l_paddingBuffer[256];

        while((l_padding--) > 0) {
            if(p_padWithZeros) {
                l_paddingBuffer[kstrlen(l_paddingBuffer)] = '0';
            } else {
                l_paddingBuffer[kstrlen(l_paddingBuffer)] = ' ';
            }
        }

        kstrcat(l_paddingBuffer, p_buffer);
        kstrcpy(p_buffer, l_paddingBuffer);
    }

    return p_buffer;
}
