#include <stdint.h>

#include "common.h"
#include "kernel/arch/x86/assembly.h"
#include "kernel/arch/x86/bios.h"
#include "kernel/arch/x86/mm/vmm.h"
#include "kernel/libk/stdlib.h"
#include "kernel/libk/string.h"
#include "kernel/sys/dev/tty.h"

typedef struct {
    tty_driver_t driver;
    int width;
    int height;
    int cursorX;
    int cursorY;
    uint8_t attr;
    void *buffer;
} vgaconsole_driver_t;

void vgaconsole_init();
static void vgaconsole_write(tty_driver_t *driver, const void *str, size_t n);
static inline void vgaconsole_readCursorPosition(vgaconsole_driver_t *driver);
static inline void vgaconsole_writeCursorPosition(vgaconsole_driver_t *driver);

void vgaconsole_init() {
    vgaconsole_driver_t *driver = malloc(sizeof(vgaconsole_driver_t));

    driver->driver.write = vgaconsole_write;
    driver->width = 80;
    driver->height = 25;
    driver->attr = 0x07;
    driver->buffer = vmm_map((const void *)0xb8000, 1, true);

    vgaconsole_readCursorPosition(driver);

    tty_driver_register((tty_driver_t *)driver);
}

static void vgaconsole_write(tty_driver_t *driver, const void *str, size_t n) {
    vgaconsole_driver_t *drv = (vgaconsole_driver_t *)driver;
    
    for(size_t i = 0; i < n; i++) {
        char c = ((const char *)str)[i];

        if(c != 0) {
            if(c == '\t') {
                drv->cursorX += 4 - (drv->cursorX % 4);
            } else if(c == '\n') {
                drv->cursorX = 0;
                drv->cursorY++;
            } else if(c == '\r') {
                drv->cursorX = 0;
            } else {
                int bufferIndex = (drv->cursorY * drv->width + drv->cursorX) * 2;

                ((uint8_t *)drv->buffer)[bufferIndex] = c;
                ((uint8_t *)drv->buffer)[bufferIndex + 1] = drv->attr;

                drv->cursorX++;
            }

            if(drv->cursorX >= drv->width) {
                drv->cursorX = 0;
                drv->cursorY++;
            }

            if(drv->cursorY >= drv->height) {
                int lineCount = drv->cursorY - drv->height + 1;

                if(lineCount >= drv->height - 1) {
                    drv->cursorX = 0;
                    drv->cursorY = 0;

                    for(int i = 0; i < drv->width * drv->height; i++) {
                        ((uint16_t *)drv->buffer)[i] = 0x0700;
                    }
                } else {
                    size_t offset = drv->width * lineCount * 2;
                    size_t totalLength = drv->width * drv->height * 2;
                    size_t copyLength = totalLength - offset;

                    memcpy(drv->buffer, (uint8_t *)drv->buffer + offset, copyLength);
                    memset((uint8_t *)drv->buffer + copyLength, 0, offset);

                    drv->cursorY -= lineCount;
                }
            }
        }
    }

    vgaconsole_writeCursorPosition(drv);
}

static inline void vgaconsole_readCursorPosition(vgaconsole_driver_t *driver) {
    uint16_t position;

    outb(0x3d4, 0x0f);
    position = inb(0x3d5);
    outb(0x3d4, 0x0e);
    position |= inb(0x3d5) << 8;

    driver->cursorY = position / driver->width;
    driver->cursorX = position % driver->width;
}

static inline void vgaconsole_writeCursorPosition(vgaconsole_driver_t *driver) {
    uint16_t position = driver->cursorY * driver->width + driver->cursorX;

    outb(0x3d4, 0x0f);
    outb(0x3d5, position);
    outb(0x3d4, 0x0e);
    outb(0x3d5, position >> 8);
}
