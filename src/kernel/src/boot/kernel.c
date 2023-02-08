#include <stdbool.h>

#include "boot/boot.h"
#include "dev/acpi.h"
#include "dev/debugcon.h"
#include "dev/parallel.h"
#include "dev/serial.h"
#include "debug.h"

void main(const struct ts_boot *p_boot) {
    struct ts_devDebugcon l_debugcon = {
        .a_basePort = 0xe9
    };

    debugInit((t_debugWriteFunc)debugconPutc, &l_debugcon);

    struct ts_devSerial l_com1 = {
        .a_basePort = 0x3f8
    };

    if(serialInit(&l_com1) != 0) {
        debugPrint("serial: COM1 initialization failed.\n");
    } else {
        debugPrint("serial: COM1 initialization success.\n");
        debugInit((t_debugWriteFunc)serialSend, &l_com1);
    }

    struct ts_devParallel l_lpt1 = {
        .a_basePort = 0x378
    };

    if(parallelInit(&l_lpt1) != 0) {
        debugPrint("parallel: LPT1 initialization failed.\n");
    } else {
        debugPrint("parallel: LPT1 initialization success.\n");
        debugInit((t_debugWriteFunc)parallelSend, &l_lpt1);
    }

    acpiInit();

    while(true) {
        asm("hlt");
    }
}
