#include <stdbool.h>

#include <kernel/arch/arch.h>
#include <kernel/boot/boot.h>
#include <kernel/dev/drivers/ps2kbd.h>
#include <kernel/dev/drivers/ps2mouse.h>
#include <kernel/dev/device.h>
#include <kernel/dev/framebuffer.h>
#include <kernel/dev/terminal.h>
#include <kernel/dev/timer.h>
#include <kernel/fonts/fonts.h>
#include <kernel/debug.h>

static void registerDrivers(void);

static struct ts_devTerminal s_terminal;
static struct ts_devFramebufferFont *s_font = &g_font8;

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

    debug("kernel: Starting AnkeKernel...\n");

    if(archPreinit(p_boot) != 0) {
        debug("kernel: Architecture-specific initialization failed.\n");
        debug("kernel: System halted.\n");
        archHaltAndCatchFire();
    }

    // Here, memory management is available.

    if(deviceInit() != 0) {
        debug("kernel: Failed to initialize device driver list.\n");
        debug("kernel: System halted.\n");
        archHaltAndCatchFire();
    }

    // Here, driver list is empty but available for use.
    registerDrivers();

    if(archInit() != 0) {
        debug("kernel: Architecture-specific initialization failed.\n");
        debug("kernel: System halted.\n");
        archHaltAndCatchFire();
    }

    // Here, basic device drivers are loaded, but a lot of devices should still
    // have an "unknown" driver.

    /* TODO: load device drivers and try to find a driver for each unknown
    device. */

    debug("kernel: Done.\n");

    while(true) {
        archHalt();
    }
}

static void registerDrivers(void) {
    debug("kernel: Registering drivers...\n");
    deviceRegisterDriver((const struct ts_deviceDriver *)&g_deviceDriverPs2Kbd);
    deviceRegisterDriver((const struct ts_deviceDriver *)&g_deviceDriverPs2Mouse);
}
