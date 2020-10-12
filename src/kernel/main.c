#include <stdbool.h>
#include <stdint.h>

int kernel_main(void) {
    uint8_t *videoBuffer = (uint8_t *)0xc00b8000;

    videoBuffer[0] = '*';
    videoBuffer[1] = 0x70;

    while(true) {
        asm("cli\n \
             hlt");
    }
}