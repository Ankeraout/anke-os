#include <stdbool.h>

#include "arch/x86/gdt.h"
#include "arch/x86/idt.h"
#include "arch/x86/inline.h"
#include "arch/x86/pic.h"
#include "arch/x86/pit.h"
#include "boot/boot.h"
#include "dev/acpi.h"
#include "dev/debugcon.h"
#include "dev/ide.h"
#include "dev/parallel.h"
#include "dev/pci.h"
#include "dev/serial.h"
#include "debug.h"

static struct ts_devDebugcon s_kernelDebugcon = {
    .a_basePort = 0xe9
};

void main(const struct ts_boot *p_boot) {
    debugPrint("kernel: Starting AnkeKernel...\n");

    debugInit((t_debugWriteFunc)debugconPutc, &s_kernelDebugcon);

    gdtInit();
    idtInit();
    picInit();
    pitInit();
    acpiInit();
    pciInit();

    struct ts_devIde l_ide1 = {
        .a_ioPortBase = 0x1f0,
        .a_ioPortControl = 0x3f6
    };

    struct ts_devIde l_ide2 = {
        .a_ioPortBase = 0x170,
        .a_ioPortControl = 0x376
    };

    ideInit(&l_ide1);
    ideInit(&l_ide2);

    debugPrint("kernel: Initialization complete.\n");

    while(true) {
        hlt();
    }
}
