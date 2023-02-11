#include <stdbool.h>

#include "arch/x86/gdt.h"
#include "arch/x86/idt.h"
#include "arch/x86/inline.h"
#include "arch/x86/pic.h"
#include "arch/x86/pit.h"
#include "arch/x86/ps2.h"
#include "boot/boot.h"
#include "dev/acpi.h"
#include "dev/debugcon.h"
#include "dev/framebuffer.h"
#include "dev/ide.h"
#include "dev/parallel.h"
#include "dev/pci.h"
#include "dev/serial.h"
#include "dev/terminal.h"
#include "fonts/fonts.h"
#include "debug.h"

static struct ts_devDebugcon s_kernelDebugcon = {
    .a_basePort = 0xe9
};

void main(struct ts_boot *p_boot) {
    struct ts_devFramebufferFont *l_font = &g_font16;

    struct ts_devTerminal l_terminal = {
        .a_x = 0,
        .a_y = 0,
        .a_width = p_boot->a_framebuffer.a_width / 8,
        .a_height = p_boot->a_framebuffer.a_height / l_font->a_height,
        .a_font = l_font,
        .a_framebuffer = &p_boot->a_framebuffer,
        .a_foregroundColor = 0xffffffff,
        .a_backgroundColor = 0x00000000
    };

    debugInit((tf_debugWriteFunc *)terminalPutc, &l_terminal);

    debugPrint("kernel: Starting AnkeKernel...\n");

    gdtInit();
    idtInit();
    picInit();

    // Enable interrupts
    sti();
    debugPrint("kernel: Interrupts enabled.\n");

    pitInit();
    acpiInit();

    if(acpiIsPs2ControllerPresent()) {
        ps2Init();
    } else {
        debugPrint("kernel: PS/2 port is not present according to ACPI.\n");
    }

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
