// =============================================================================
// File inclusion
// =============================================================================
#include <stdarg.h>

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
    return 0;
}
