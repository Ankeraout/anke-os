#include <stdbool.h>
#include "boot/loader/arch/x86/asm.h"
#include "boot/loader/drivers/isadma.h"
#include "boot/loader/string.h"

extern int g_isadmaBuffer;

void isadma_init(int p_channel, size_t p_size) {
    outb(0x0a, 0x04 | (p_channel & 0x03)); // Mask channel
    outb(0x0c, 0xff); // Reset the master flip-flop
    outb(0x04, (uint32_t)&g_isadmaBuffer); // Address = 0x....yy
    outb(0x04, ((uint32_t)&g_isadmaBuffer) >> 8); // Address = 0x..yy..
    outb(0x0c, 0xff); // Reset the master flip-flop
    outb(0x05, p_size - 1); // Size = 0x..yy
    outb(0x05, (p_size - 1) >> 8); // Size = 0xyy..
    outb(0x81, ((uint32_t)&g_isadmaBuffer) >> 16); // Address = 0xyy....
    outb(0x0a, p_channel & 0x03); // Unmask channel
}

void isadma_init_read(int p_channel) {
    outb(0x0a, 0x04 | (p_channel & 0x03)); // Mask channel
    outb(0x0b, 0x56);
    outb(0x0a, p_channel & 0x03); // Unmask channel
}

void isadma_read(void *p_buffer, size_t p_size) {
    memcpy(p_buffer, &g_isadmaBuffer, p_size);
}

void isadma_init_write(int p_channel, void *p_buffer, size_t p_size) {
    memcpy(&g_isadmaBuffer, p_buffer, p_size);
    outb(0x0a, 0x04 | (p_channel & 0x03)); // Mask channel
    outb(0x0b, 0x5a);
    outb(0x0a, p_channel & 0x03); // Unmask channel
}
