#include <stddef.h>
#include <stdint.h>

#include "kernel/arch/x86_64/asm.h"
#include "kernel/arch/x86_64/isr.h"
#include "kernel/arch/x86_64/pic.h"
#include "kernel/printk.h"
#include "kernel/string.h"

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
    if(p_registers->m_interruptNumber < 48) {
        tf_isrHandler *l_handler = s_isrHandlers[p_registers->m_interruptNumber];
        void *l_handlerArg = s_isrHandlerArgs[p_registers->m_interruptNumber];

        if(p_registers->m_interruptNumber < 32) {
            if(l_handler == NULL) {
                printk(
                    "panic: Unhandled CPU exception %d, code: 0x%016x.\n",
                    p_registers->m_interruptNumber,
                    p_registers->m_errorCode
                );

                panic(p_registers);
            } else {
                l_handler(p_registers, l_handlerArg);
            }
        } else if(p_registers->m_interruptNumber < 48) {
            if(l_handler != NULL) {
                l_handler(p_registers, l_handlerArg);
            }

            picEndOfInterrupt(p_registers->m_interruptNumber - 32);
        } else {
            printk(
                "panic: Unhandled interrupt %d\n",
                p_registers->m_interruptNumber
            );

            panic(p_registers);
        }
    } else {
        printk(
            "panic: Unhandled interrupt %d\n",
            p_registers->m_interruptNumber
        );

        panic(p_registers);
    }
}

static void panic(const struct ts_isrRegisters *p_registers) {
    printk(
        "panic: RIP=0x%016lx RSP=0x%016lx RBP=0x%016lx\n",
        p_registers->m_rip,
        p_registers->m_rsp,
        p_registers->m_rbp
    );

    printk(
        "panic: RSI=0x%016lx RDI=0x%016lx RAX=0x%016lx\n",
        p_registers->m_rsi,
        p_registers->m_rdi,
        p_registers->m_rax
    );

    printk(
        "panic: RBX=0x%016lx RCX=0x%016lx RDX=0x%016lx\n",
        p_registers->m_rbx,
        p_registers->m_rcx,
        p_registers->m_rdx
    );

    printk(
        "panic: R8 =0x%016lx R9 =0x%016lx R10=0x%016lx\n",
        p_registers->m_r8,
        p_registers->m_r9,
        p_registers->m_r10
    );

    printk(
        "panic: R11=0x%016lx R12=0x%016lx R13=0x%016lx\n",
        p_registers->m_r11,
        p_registers->m_r12,
        p_registers->m_r13
    );

    printk(
        "panic: R14=0x%016lx R15=0x%016lx RFLAGS=0x%016lx\n",
        p_registers->m_r14,
        p_registers->m_r15,
        p_registers->m_rflags
    );

    printk(
        "panic: CS=0x%04x DS=0x%04x ES=0x%04x FS=0x%04x GS=0x%04x SS=0x%04x\n",
        p_registers->m_cs & 0xffff,
        p_registers->m_ds & 0xffff,
        p_registers->m_es & 0xffff,
        p_registers->m_fs & 0xffff,
        p_registers->m_gs & 0xffff,
        p_registers->m_ss & 0xffff
    );

    printk("panic: System halted.\n");

    cli();
    hlt();
}
