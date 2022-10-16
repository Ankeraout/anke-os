#include <stdbool.h>

#include "boot/boot.h"

void main(const struct ts_boot *p_boot) {
    while(true) {
        asm("hlt");
    }
}
