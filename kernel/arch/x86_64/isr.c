#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/arch/x86_64/inline.h>
#include <kernel/arch/x86_64/isr.h>
#include <kernel/debug.h>

static tf_isrHandler *s_isrHandlers[48];
static void *s_isrHandlerArgs[48];

static void panic(const struct ts_isrRegisters *p_registers);

void isrInit(void) {
    memset(s_isrHandlers, 0, sizeof(s_isrHandlers));
    memset(s_isrHandlerArgs, 0, sizeof(s_isrHandlerArgs));
}

void isrSetHandler(int p_interruptNumber, tf_isrHandler *p_handler, void *p_arg) {
    if((p_interruptNumber < 0) || (p_interruptNumber >= 48)) {
        return;
    }

    s_isrHandlers[p_interruptNumber] = p_handler;
    s_isrHandlerArgs[p_interruptNumber] = p_arg;
}

void isrHandler(struct ts_isrRegisters *p_registers) {
    if(p_registers->a_interruptNumber < 48) {
        tf_isrHandler *l_handler = s_isrHandlers[p_registers->a_interruptNumber];
        void *l_handlerArg = s_isrHandlerArgs[p_registers->a_interruptNumber];

        if(p_registers->a_interruptNumber < 32) {
            if(l_handler == NULL) {
                debug(
                    "panic: Unhandled CPU exception %d, code: 0x%016x.\n",
                    p_registers->a_interruptNumber,
                    p_registers->a_errorCode
                );

                panic(p_registers);
            } else {
                l_handler(p_registers, l_handlerArg);
            }
        } else if(p_registers->a_interruptNumber < 48) {
            if(l_handler != NULL) {
                l_handler(p_registers, l_handlerArg);
            }

            // TODO: send EOI
        } else {
            debug(
                "panic: Unhandled interrupt %d\n",
                p_registers->a_interruptNumber
            );

            panic(p_registers);
        }
    } else {
        debug(
            "panic: Unhandled interrupt %d\n",
            p_registers->a_interruptNumber
        );

        panic(p_registers);
    }
}

static void panic(const struct ts_isrRegisters *p_registers) {
    debug(
        "panic: RIP=0x%016x RSP=0x%016x RBP=0x%016x\n",
        p_registers->a_rip,
        p_registers->a_rsp,
        p_registers->a_rbp
    );

    debug(
        "panic: RSI=0x%016x RDI=0x%016x RAX=0x%016x\n",
        p_registers->a_rsi,
        p_registers->a_rdi,
        p_registers->a_rax
    );

    debug(
        "panic: RBX=0x%016x RCX=0x%016x RDX=0x%016x\n",
        p_registers->a_rbx,
        p_registers->a_rcx,
        p_registers->a_rdx
    );

    debug(
        "panic: R8 =0x%016x R9 =0x%016x R10=0x%016x\n",
        p_registers->a_r8,
        p_registers->a_r9,
        p_registers->a_r10
    );

    debug(
        "panic: R11=0x%016x R12=0x%016x R13=0x%016x\n",
        p_registers->a_r11,
        p_registers->a_r12,
        p_registers->a_r13
    );

    debug(
        "panic: R14=0x%016x R15=0x%016x RFLAGS=0x%016x\n",
        p_registers->a_r14,
        p_registers->a_r15,
        p_registers->a_rflags
    );

    debug(
        "panic: CS=0x%04x DS=0x%04x ES=0x%04x FS=0x%04x GS=0x%04x SS=0x%04x\n",
        p_registers->a_cs,
        p_registers->a_ds,
        p_registers->a_es,
        p_registers->a_fs,
        p_registers->a_gs,
        p_registers->a_ss
    );

    debug("panic: System halted.\n");

    cli();
    hlt();
}
