#include <stddef.h>
#include <stdint.h>

#include "arch/x86/inline.h"
#include "arch/x86/isr.h"
#include "arch/x86/dev/i8259.h"
#include "debug.h"

static tf_isrHandler *s_isrHandlers[48] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static void panic(const struct ts_isrRegisters *p_registers);

void isrSetHandler(int p_interruptNumber, tf_isrHandler *p_handler) {
    if((p_interruptNumber < 0) || (p_interruptNumber >= 48)) {
        return;
    }

    s_isrHandlers[p_interruptNumber] = p_handler;
}

void isrHandler(struct ts_isrRegisters *p_registers) {
    if(p_registers->a_interruptNumber < 48) {
        tf_isrHandler *l_handler = s_isrHandlers[p_registers->a_interruptNumber];

        if(p_registers->a_interruptNumber < 32) {
            if(l_handler == NULL) {
                debugPrint("panic: Unhandled CPU exception 0x");
                debugPrintHex8(p_registers->a_interruptNumber);
                debugPrint(", code=0x");
                debugPrintHex64(p_registers->a_errorCode);
                debugPrint(".\n");

                panic(p_registers);
            } else {
                l_handler(p_registers);
            }
        } else if(p_registers->a_interruptNumber < 48) {
            if(l_handler != NULL) {
                l_handler(p_registers);
            }

            i8259EndOfInterrupt(p_registers->a_interruptNumber >= 40);
        } else {
            debugPrint("panic: Unhandled interrupt 0x");
            debugPrintHex8(p_registers->a_interruptNumber);
            debugPrint(".\n");

            panic(p_registers);
        }
    } else {

    }
}

static void panic(const struct ts_isrRegisters *p_registers) {
    debugPrint("panic: RAX=0x");
    debugPrintHex64(p_registers->a_rax);
    debugPrint(" RBX=0x");
    debugPrintHex64(p_registers->a_rbx);
    debugPrint(" RCX=0x");
    debugPrintHex64(p_registers->a_rcx);
    debugPrint("\npanic: RDX=0x");
    debugPrintHex64(p_registers->a_rdx);
    debugPrint(" RSI=0x");
    debugPrintHex64(p_registers->a_rsi);
    debugPrint(" RDI=0x");
    debugPrintHex64(p_registers->a_rdi);
    debugPrint("\npanic: RSP=0x");
    debugPrintHex64(p_registers->a_rsp);
    debugPrint(" RBP=0x");
    debugPrintHex64(p_registers->a_rbp);
    debugPrint(" R08=0x");
    debugPrintHex64(p_registers->a_r8);
    debugPrint("\npanic: R09=0x");
    debugPrintHex64(p_registers->a_r9);
    debugPrint(" R10=0x");
    debugPrintHex64(p_registers->a_r10);
    debugPrint(" R11=0x");
    debugPrintHex64(p_registers->a_r11);
    debugPrint("\npanic: R12=0x");
    debugPrintHex64(p_registers->a_r12);
    debugPrint(" R13=0x");
    debugPrintHex64(p_registers->a_r13);
    debugPrint(" R14=0x");
    debugPrintHex64(p_registers->a_r14);
    debugPrint("\npanic: RIP=0x");
    debugPrintHex64(p_registers->a_rip);
    debugPrint(" RFLAGS=0x");
    debugPrintHex64(p_registers->a_rflags);
    debugPrint(" CS=0x");
    debugPrintHex16(p_registers->a_cs);
    debugPrint(" DS=0x");
    debugPrintHex16(p_registers->a_ds);
    debugPrint("\npanic: ES=0x");
    debugPrintHex16(p_registers->a_es);
    debugPrint(" FS=0x");
    debugPrintHex16(p_registers->a_fs);
    debugPrint(" GS=0x");
    debugPrintHex16(p_registers->a_gs);
    debugPrint(" SS=0x");
    debugPrintHex16(p_registers->a_ss);
    debugPrint("\n");

    debugPrint("panic: System halted.\n");

    cli();
    hlt();
}
