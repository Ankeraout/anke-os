#include <stdbool.h>

#include "boot/boot.h"
#include "dev/acpi.h"
#include "dev/debugcon.h"

void main(const struct ts_boot *p_boot) {
    debugconPuts("Hello world!\n");
    acpiInit();

    while(true) {
        asm("hlt");
    }
}
