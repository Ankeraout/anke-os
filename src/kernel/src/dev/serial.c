#include <stdbool.h>
#include <stdint.h>

#include "arch/x86/inline.h"
#include "dev/serial.h"

static inline bool serialIsTransmitBufferEmpty(struct ts_devSerial *p_dev);
static inline bool serialIsReceiveBufferEmpty(struct ts_devSerial *p_dev);

int serialInit(struct ts_devSerial *p_dev) {
    outb(p_dev->a_basePort + 1, 0x00); // Disable all interrupts
    outb(p_dev->a_basePort + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(p_dev->a_basePort, 0x03); // Set divisor to 3 (38400 bauds)
    outb(p_dev->a_basePort + 1, 0x00); // (high byte)
    outb(p_dev->a_basePort + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(p_dev->a_basePort + 2, 0xc7); // Enable FIFO, 14 bytes threshold
    outb(p_dev->a_basePort + 4, 0x0b); // IRQs enabled, RTS/DSR set
    outb(p_dev->a_basePort + 4, 0x1e); // Set loopback mode for testing

    outb(p_dev->a_basePort, 0xae); // Send 0xae

    if(inb(p_dev->a_basePort) != 0xae) {
        return 1;
    }

    outb(p_dev->a_basePort + 4, 0x0f); // Disable loopback mode

    return 0;
}

void serialSend(struct ts_devSerial *p_dev, uint8_t p_value) {
    while(!serialIsTransmitBufferEmpty(p_dev));
    
    outb(p_dev->a_basePort, p_value);
}

uint8_t serialReceive(struct ts_devSerial *p_dev) {
    while(serialIsReceiveBufferEmpty(p_dev));

    return inb(p_dev->a_basePort);
}

static inline bool serialIsTransmitBufferEmpty(struct ts_devSerial *p_dev) {
    return (inb(p_dev->a_basePort + 5) & 0x20) != 0;
}

static inline bool serialIsReceiveBufferEmpty(struct ts_devSerial *p_dev) {
    return (inb(p_dev->a_basePort + 5) & 0x01) == 0;
}
