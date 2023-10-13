#include "kernel/module.h"
#include "klibc/debug.h"

int init(void) {
    kernelDebug("hello: Hello world!\n");
}

void exit(void) {
    kernelDebug("hello: Exiting!\n");
}

M_DECLARE_MODULE("hello", init, exit);
