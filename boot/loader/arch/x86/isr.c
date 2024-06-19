#include <stdint.h>

#include "isr.h"
#include "string.h"

static tf_interruptHandler *s_isrHandlers[48];
static void *s_isrHandlerArgs[48];

void isrHandler(struct ts_isrRegisters *p_registers) {
    const uint32_t l_interruptNumber = p_registers->m_interruptNumber;

    if(l_interruptNumber >= 48) {
        return;
    }

    tf_interruptHandler *l_interruptHandler =
        s_isrHandlers[l_interruptNumber];

    if(l_interruptNumber != NULL) {
        l_interruptHandler(s_isrHandlerArgs[l_interruptNumber]);
    }
}

void isrInit(void) {
    memset(s_isrHandlers, 0, sizeof(s_isrHandlers));
}

void isrAdd(int p_interrupt, tf_interruptHandler *p_handler, void *p_arg) {
    if(p_interrupt < 0 || p_interrupt >= 48) {
        return;
    }

    s_isrHandlers[p_interrupt] = p_handler;
    s_isrHandlerArgs[p_interrupt] = p_arg;
}

void isrRemove(int p_interrupt, tf_interruptHandler *p_handler, void *p_arg) {
    if(p_interrupt < 0 || p_interrupt >= 48) {
        return;
    }

    s_isrHandlers[p_interrupt] = NULL;
}
