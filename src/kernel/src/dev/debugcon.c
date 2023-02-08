#include <stddef.h>

#include "arch/x86/inline.h"
#include "dev/debugcon.h"

void debugconPutc(struct ts_devDebugcon *p_dev, uint8_t p_character) {
    outb(p_dev->a_basePort, p_character);
}
