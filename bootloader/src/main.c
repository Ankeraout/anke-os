#include <stdint.h>

#include "stdio.h"

int main() {
    stdio_init();

    puts("Hello from 64-bit mode.\n");

    while(1);

    return 0;
}
