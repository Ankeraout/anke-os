#include <stdbool.h>

#include "boot/boot.h"
#include "dev/acpi.h"
#include "dev/debugcon.h"
#include "dev/parallel.h"
#include "dev/pci.h"
#include "dev/serial.h"
#include "debug.h"

void main(const struct ts_boot *p_boot) {
    struct ts_devDebugcon l_debugcon = {
        .a_basePort = 0xe9
    };

    debugInit((t_debugWriteFunc)debugconPutc, &l_debugcon);

    acpiInit();
    pciInit();

    while(true) {
        asm("hlt");
    }
}
