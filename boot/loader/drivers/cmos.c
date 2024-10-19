#include <stdbool.h>
#include "boot/loader/arch/x86/asm.h"
#include "boot/loader/drivers/cmos.h"

#define C_CMOS_IO_PORT_REGISTER_SELECT 0x70
#define C_CMOS_IO_PORT_DATA 0x71

uint8_t cmos_read(uint8_t p_register) {
    outb(C_CMOS_IO_PORT_REGISTER_SELECT, p_register & 0x7fU);
    return inb(C_CMOS_IO_PORT_DATA);
}
