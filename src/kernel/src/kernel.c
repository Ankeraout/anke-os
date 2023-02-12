#include <stdbool.h>

#include "arch/x86/gdt.h"
#include "arch/x86/idt.h"
#include "arch/x86/inline.h"
#include "boot/boot.h"
#include "dev/acpi.h"
#include "dev/device.h"
#include "dev/framebuffer.h"
#include "dev/terminal.h"
#include "fonts/fonts.h"
#include "debug.h"

static struct ts_devTerminal s_terminal;
static struct ts_devFramebufferFont *s_font = &g_font8;
static struct ts_device s_acpiDevice = {
    .a_driver = &g_acpiDriver,
    .a_parent = NULL,
    .a_driverData = NULL
};

void main(struct ts_boot *p_boot) {
    s_terminal.a_x = 0;
    s_terminal.a_y = 0;
    s_terminal.a_width = p_boot->a_framebuffer.a_width / 8;
    s_terminal.a_height = p_boot->a_framebuffer.a_height / s_font->a_height;
    s_terminal.a_font = s_font;
    s_terminal.a_framebuffer = &p_boot->a_framebuffer;
    s_terminal.a_foregroundColor = 0xffaaaaaa;
    s_terminal.a_backgroundColor = 0xff000000;

    debugInit((tf_debugWriteFunc *)terminalPutc, &s_terminal);

    framebufferFillRectangle(&p_boot->a_framebuffer, NULL, s_terminal.a_backgroundColor);

    debugPrint("kernel: Starting AnkeKernel...\n");

    gdtInit();
    idtInit();
    sti();
    debugPrint("kernel: Interrupts enabled.\n");

    if(s_acpiDevice.a_driver->a_init(&s_acpiDevice)) {
        debugPrint("kernel: ACPI driver initialization failed.\n");
        debugPrint("kernel: System halted.\n");

        while(true) {
            cli();
            hlt();
        }
    }

    debugPrint("kernel: Done.\n");

    while(true) {
        hlt();
    }
}
