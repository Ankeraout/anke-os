#include <stdbool.h>

#include "kernel/arch/x86_64/inline.h"
#include "kernel/arch.h"
#include "kernel/boot.h"
#include "klibc/debug.h"

static inline void halt(void);
static int kernelPreinit(void);
static int kernelInit(void);

void kernelMain(const struct ts_kernelBootInfo *p_bootInfo) {
    int l_returnValue = archPreinit(p_bootInfo);

    if(l_returnValue != 0) {
        kernelDebug("archPreinit() failed with code %d.\n", l_returnValue);
        halt();
    }

    l_returnValue = kernelPreinit();

    if(l_returnValue != 0) {
        kernelDebug("kernelPreinit() failed with code %d.\n", l_returnValue);
        halt();
    }

    l_returnValue = archInit();

    if(l_returnValue != 0) {
        kernelDebug("archInit() failed with code %d.\n", l_returnValue);
        halt();
    }

    l_returnValue = kernelInit();

    if(l_returnValue != 0) {
        kernelDebug("kernelInit() failed with code %d.\n", l_returnValue);
        halt();
    }
}

static inline void halt(void) {
    while(true) {
        cli();
        hlt();
    }
}

static int kernelPreinit(void) {
    kernelDebug("kernelPreinit() called.\n");

    return 0;
}

static int kernelInit(void) {
    kernelDebug("kernelInit() called.\n");

    return 0;
}
