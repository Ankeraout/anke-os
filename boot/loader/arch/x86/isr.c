#include <stddef.h>
#include <stdint.h>

#include "boot/loader/arch/x86/asm.h"
#include "boot/loader/arch/x86/isr.h"
#include "boot/loader/arch/x86/pic.h"
#include "boot/loader/stdio.h"
#include "boot/loader/string.h"

static struct {
    tf_isrHandler *handler;
    void *arg;
} s_isrHandlers[48];

void isr_init(void) {
    memset(s_isrHandlers, 0, sizeof(s_isrHandlers));
}

void isr_setHandler(int p_interruptNumber, tf_isrHandler *p_handler, void *p_arg) {
    if((p_interruptNumber < 0) || (p_interruptNumber >= 48)) {
        return;
    }

    s_isrHandlers[p_interruptNumber].handler = p_handler;
    s_isrHandlers[p_interruptNumber].arg = p_arg;
}

void isr_handler(struct ts_isrRegisters *p_registers) {
    if(p_registers->interruptNumber < 48) {
        tf_isrHandler *l_handler =
            s_isrHandlers[p_registers->interruptNumber].handler;
        void *l_handlerArg = s_isrHandlers[p_registers->interruptNumber].arg;

        if(p_registers->interruptNumber < 32) {
            if(l_handler == NULL) {
                printf(
                    "isr: Unhandled CPU exception %d, code: 0x%016x.\n",
                    p_registers->interruptNumber,
                    p_registers->errorCode
                );
            } else {
                l_handler(p_registers, l_handlerArg);
            }
        } else if(p_registers->interruptNumber < 48) {
            if(l_handler != NULL) {
                l_handler(p_registers, l_handlerArg);
            }

            pic_endOfInterrupt(p_registers->interruptNumber - 32);
        } else {
            printf(
                "isr: Unhandled interrupt %d\n",
                p_registers->interruptNumber
            );
        }
    } else {
        printf(
            "isr: Unhandled interrupt %d\n",
            p_registers->interruptNumber
        );
    }
}
