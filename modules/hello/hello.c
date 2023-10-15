#include "kernel/module.h"
#include "klibc/debug.h"

int init(void) {
    kernelDebug("hello: Hello world!\n");
    return 0;
}

void exit(void) {
    kernelDebug("hello: Exiting!\n");
}

M_DECLARE_MODULE("hello", init, exit);
