#include <stdint.h>

#include "arch/x86/inline.h"
#include "arch/x86/isr.h"
#include "common.h"
#include "debug.h"

#define C_IOPORT_PIT_C0_DATA 0x40
#define C_IOPORT_PIT_C1_DATA 0x41
#define C_IOPORT_PIT_C2_DATA 0x42
#define C_IOPORT_PIT_CMD 0x43
#define C_PIT_FREQUENCY 1193182

static volatile uint64_t s_pitCounter;

static void pitInterruptHandler(struct ts_isrRegisters *p_registers);

void pitInit(void) {
    // Register interrupt handler for IRQ 0
    isrSetHandler(32, pitInterruptHandler);

    // Reinitialize counter
    s_pitCounter = 0;

    // Configure the PIT to send an IRQ every 10ms
    const int l_clocksPerIrq = C_PIT_FREQUENCY / 100;

    // Channel 0: regular IRQ
    outb(C_IOPORT_PIT_CMD, 0x36);
    iowait();
    outb(C_IOPORT_PIT_C0_DATA, l_clocksPerIrq);
    iowait();
    outb(C_IOPORT_PIT_C0_DATA, l_clocksPerIrq >> 8);
    iowait();

    // Channels 1 and 2 are unused

    debugPrint("pit: PIT initialized.\n");
}

void pitSleep(uint64_t p_milliseconds) {
    const uint64_t l_initialCounter = s_pitCounter;
    const uint64_t l_ticks = (p_milliseconds + 9) / 10;

    while((s_pitCounter - l_initialCounter) < l_ticks) {
        hlt();
    }
}

uint64_t pitGetTimeMilliseconds(void) {
    return s_pitCounter * 10;
}

static void pitInterruptHandler(struct ts_isrRegisters *p_registers) {
    M_UNUSED_PARAMETER(p_registers);

    s_pitCounter++;
}
