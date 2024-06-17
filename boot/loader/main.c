#include "asm.h"
#include "bioscall.h"
#include "stdio.h"

void main(void) {
    stdio_init();

    printf("Hello from protected mode.\n");

    while(1) {
        cli();
        hlt();
    }
}
