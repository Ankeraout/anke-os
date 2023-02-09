#include <stdint.h>

#include "arch/x86/inline.h"
#include "debug.h"

#define C_IOPORT_PIT_C0_DATA 0x40
#define C_IOPORT_PIT_C1_DATA 0x41
#define C_IOPORT_PIT_C2_DATA 0x42
#define C_IOPORT_PIT_CMD 0x43
#define C_PIT_FREQUENCY 1193182

void pitInit(void) {
    // Configure the PIT to send an IRQ every 10ms
    const int l_clocksPerIrq = C_PIT_FREQUENCY / 100;

    // Channel 0: regular IRQ
    outb(C_IOPORT_PIT_CMD, 0x36);
    iowait();
    outb(C_IOPORT_PIT_C0_DATA, l_clocksPerIrq);
    iowait();
    outb(C_IOPORT_PIT_C0_DATA, l_clocksPerIrq >> 16);
    iowait();

    // Channels 1 and 2 are unused

    debugPrint("pit: PIT initialized.\n");
}
