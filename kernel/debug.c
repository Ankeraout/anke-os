
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <kernel/debug.h>

enum te_parserState {
    E_PARSERSTATE_START,
    E_PARSERSTATE_ESCAPE,
    E_PARSERSTATE_MINLEN,
    E_PARSERSTATE_MAXLEN,
    E_PARSERSTATE_ACTION_L
};

static tf_debugPrint *s_debugWriteFunc = NULL;
static void *s_debugWriteFuncParameter = NULL;

void debugInit(tf_debugPrint *p_writeFunc, void *p_parameter) {
    s_debugWriteFunc = p_writeFunc;
    s_debugWriteFuncParameter = p_parameter;
}

void debug(const char *p_format, ...) {
    va_list l_args;
    va_start(l_args, p_format);

    char l_buffer[512];

    vsnprintf(l_buffer, 512, p_format, l_args);

    va_end(l_args);

    s_debugWriteFunc(s_debugWriteFuncParameter, l_buffer);
}
