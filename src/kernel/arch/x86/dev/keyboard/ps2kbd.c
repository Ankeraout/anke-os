#include <stdbool.h>

#include "kernel/arch/x86/assembly.h"
#include "kernel/libk/stdlib.h"
#include "kernel/sys/interrupt.h"
#include "kernel/sys/dev/keyboard.h"

typedef struct {
    keyboard_driver_t driver;
    uint8_t lastWrittenByte;
    uint8_t buffer[2];
    int bufferLength;
} ps2kbd_driver_t;

void ps2kbd_init();
static void ps2kbd_setLeds(keyboard_driver_t *driver, int leds);
static void ps2kbd_interruptHandler(ps2kbd_driver_t *driver);
static void ps2kbd_writeByte(ps2kbd_driver_t *driver, uint8_t byte);

void ps2kbd_init() {
    ps2kbd_driver_t *driver = malloc(sizeof(ps2kbd_driver_t));

    driver->driver.setLeds = ps2kbd_setLeds;
    driver->bufferLength = 0;

    interrupt_register(1, ps2kbd_interruptHandler, driver);
    keyboard_driver_register((keyboard_driver_t *)driver);
}

static void ps2kbd_setLeds(keyboard_driver_t *driver, int leds) {
    ps2kbd_writeByte((ps2kbd_driver_t *)driver, 0xed);
    ps2kbd_writeByte((ps2kbd_driver_t *)driver, leds);
}

static void ps2kbd_interruptHandler(ps2kbd_driver_t *driver) {
    uint8_t byte = inb(0x60);

    if(byte == 0xfe) {
        outb(0x60, byte);
        return;
    }

    switch(driver->bufferLength) {
        case 0:
            switch(byte) {
                case 0x01: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_ESC, true); break;
                case 0x02: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_1, true); break;
                case 0x03: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_2, true); break;
                case 0x04: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_3, true); break;
                case 0x05: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_4, true); break;
                case 0x06: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_5, true); break;
                case 0x07: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_6, true); break;
                case 0x08: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_7, true); break;
                case 0x09: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_8, true); break;
                case 0x0a: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_9, true); break;
                case 0x0b: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_0, true); break;
                case 0x0c: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_MINUS, true); break;
                case 0x0d: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_EQUAL, true); break;
                case 0x0e: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_BACKSPACE, true); break;
                case 0x0f: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_TAB, true); break;
                case 0x10: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_Q, true); break;
                case 0x11: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_W, true); break;
                case 0x12: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_E, true); break;
                case 0x13: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_R, true); break;
                case 0x14: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_T, true); break;
                case 0x15: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_Y, true); break;
                case 0x16: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_U, true); break;
                case 0x17: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_I, true); break;
                case 0x18: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_O, true); break;
                case 0x19: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_P, true); break;
                case 0x1a: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_LBRACKET, true); break;
                case 0x1b: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_RBRACKET, true); break;
                case 0x1c: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_ENTER, true); break;
                case 0x1d: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_LCTRL, true); break;
                case 0x1e: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_A, true); break;
                case 0x1f: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_S, true); break;
                case 0x20: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_D, true); break;
                case 0x21: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F, true); break;
                case 0x22: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_G, true); break;
                case 0x23: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_H, true); break;
                case 0x24: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_J, true); break;
                case 0x25: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_K, true); break;
                case 0x26: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_L, true); break;
                case 0x27: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_SEMICOLON, true); break;
                case 0x28: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_APOSTROPHE, true); break;
                case 0x29: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_BACKTICK, true); break;
                case 0x2a: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_LSHIFT, true); break;
                case 0x2b: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_BACKSLASH, true); break;
                case 0x2c: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_Z, true); break;
                case 0x2d: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_X, true); break;
                case 0x2e: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_C, true); break;
                case 0x2f: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_V, true); break;
                case 0x30: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_B, true); break;
                case 0x31: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_N, true); break;
                case 0x32: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_M, true); break;
                case 0x33: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_COMMA, true); break;
                case 0x34: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_DOT, true); break;
                case 0x35: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_SLASH, true); break;
                case 0x36: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_RSHIFT, true); break;
                case 0x37: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KPMULTIPLY, true); break;
                case 0x38: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_LALT, true); break;
                case 0x39: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_SPACE, true); break;
                case 0x3a: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_CAPSLOCK, true); break;
                case 0x3b: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F1, true); break;
                case 0x3c: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F2, true); break;
                case 0x3d: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F3, true); break;
                case 0x3e: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F4, true); break;
                case 0x3f: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F5, true); break;
                case 0x40: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F6, true); break;
                case 0x41: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F7, true); break;
                case 0x42: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F8, true); break;
                case 0x43: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F9, true); break;
                case 0x44: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F10, true); break;
                case 0x45: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_NUMLOCK, true); break;
                case 0x46: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_SCROLLLOCK, true); break;
                case 0x47: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP7, true); break;
                case 0x48: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP8, true); break;
                case 0x49: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP9, true); break;
                case 0x4a: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KPMINUS, true); break;
                case 0x4b: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP4, true); break;
                case 0x4c: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP5, true); break;
                case 0x4d: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP6, true); break;
                case 0x4e: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KPPLUS, true); break;
                case 0x4f: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP1, true); break;
                case 0x50: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP2, true); break;
                case 0x51: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP3, true); break;
                case 0x52: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP0, true); break;
                case 0x53: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KPPOINT, true); break;
                case 0x57: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F11, true); break;
                case 0x58: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F12, true); break;
                case 0x81: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_ESC, false); break;
                case 0x82: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_1, false); break;
                case 0x83: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_2, false); break;
                case 0x84: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_3, false); break;
                case 0x85: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_4, false); break;
                case 0x86: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_5, false); break;
                case 0x87: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_6, false); break;
                case 0x88: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_7, false); break;
                case 0x89: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_8, false); break;
                case 0x8a: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_9, false); break;
                case 0x8b: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_0, false); break;
                case 0x8c: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_MINUS, false); break;
                case 0x8d: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_EQUAL, false); break;
                case 0x8e: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_BACKSPACE, false); break;
                case 0x8f: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_TAB, false); break;
                case 0x90: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_Q, false); break;
                case 0x91: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_W, false); break;
                case 0x92: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_E, false); break;
                case 0x93: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_R, false); break;
                case 0x94: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_T, false); break;
                case 0x95: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_Y, false); break;
                case 0x96: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_U, false); break;
                case 0x97: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_I, false); break;
                case 0x98: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_O, false); break;
                case 0x99: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_P, false); break;
                case 0x9a: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_LBRACKET, false); break;
                case 0x9b: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_RBRACKET, false); break;
                case 0x9c: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_ENTER, false); break;
                case 0x9d: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_LCTRL, false); break;
                case 0x9e: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_A, false); break;
                case 0x9f: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_S, false); break;
                case 0xa0: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_D, false); break;
                case 0xa1: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F, false); break;
                case 0xa2: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_G, false); break;
                case 0xa3: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_H, false); break;
                case 0xa4: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_J, false); break;
                case 0xa5: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_K, false); break;
                case 0xa6: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_L, false); break;
                case 0xa7: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_SEMICOLON, false); break;
                case 0xa8: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_APOSTROPHE, false); break;
                case 0xa9: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_BACKTICK, false); break;
                case 0xaa: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_LSHIFT, false); break;
                case 0xab: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_BACKSLASH, false); break;
                case 0xac: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_Z, false); break;
                case 0xad: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_X, false); break;
                case 0xae: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_C, false); break;
                case 0xaf: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_V, false); break;
                case 0xb0: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_B, false); break;
                case 0xb1: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_N, false); break;
                case 0xb2: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_M, false); break;
                case 0xb3: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_COMMA, false); break;
                case 0xb4: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_DOT, false); break;
                case 0xb5: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_SLASH, false); break;
                case 0xb6: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_RSHIFT, false); break;
                case 0xb7: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KPMULTIPLY, false); break;
                case 0xb8: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_LALT, false); break;
                case 0xb9: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_SPACE, false); break;
                case 0xba: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_CAPSLOCK, false); break;
                case 0xbb: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F1, false); break;
                case 0xbc: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F2, false); break;
                case 0xbd: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F3, false); break;
                case 0xbe: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F4, false); break;
                case 0xbf: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F5, false); break;
                case 0xc0: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F6, false); break;
                case 0xc1: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F7, false); break;
                case 0xc2: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F8, false); break;
                case 0xc3: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F9, false); break;
                case 0xc4: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F10, false); break;
                case 0xc5: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_NUMLOCK, false); break;
                case 0xc6: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_SCROLLLOCK, false); break;
                case 0xc7: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP7, false); break;
                case 0xc8: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP8, false); break;
                case 0xc9: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP9, false); break;
                case 0xca: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KPMINUS, false); break;
                case 0xcb: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP4, false); break;
                case 0xcc: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP5, false); break;
                case 0xcd: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP6, false); break;
                case 0xce: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KPPLUS, false); break;
                case 0xcf: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP1, false); break;
                case 0xd0: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP2, false); break;
                case 0xd1: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP3, false); break;
                case 0xd2: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KP0, false); break;
                case 0xd3: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KPPOINT, false); break;
                case 0xd7: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F11, false); break;
                case 0xd8: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_F12, false); break;
                case 0xe0: driver->buffer[0] = 0xe0; driver->bufferLength = 1; break;
                case 0xe1: driver->buffer[0] = 0xe1; driver->bufferLength = 1; break;
            }

            break;

        case 1:
            if(driver->buffer[0] == 0xe0) {
                switch(byte) {
                    case 0x1c: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KPENTER, true); break;
                    case 0x1d: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_RCTRL, true); break;
                    case 0x2a: driver->buffer[1] = 0x2a; driver->bufferLength = 2; break;
                    case 0x35: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KPDIVIDE, true); break;
                    case 0x38: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_RALT, true); break;
                    case 0x47: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_HOME, true); break;
                    case 0x48: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_UP, true); break;
                    case 0x49: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_PAGEUP, true); break;
                    case 0x4b: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_LEFT, true); break;
                    case 0x4d: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_RIGHT, true); break;
                    case 0x4f: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_END, true); break;
                    case 0x50: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_DOWN, true); break;
                    case 0x51: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_PAGEDOWN, true); break;
                    case 0x52: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_INSERT, true); break;
                    case 0x53: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_DELETE, true); break;
                    case 0x5b: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_LLOGO, true); break;
                    case 0x5c: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_RLOGO, true); break;
                    case 0x5d: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_RMENU, true); break;
                    case 0x9c: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KPENTER, false); break;
                    case 0x9d: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_RCTRL, false); break;
                    case 0xb5: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_KPDIVIDE, false); break;
                    case 0xb8: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_RALT, false); break;
                    case 0xc7: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_HOME, false); break;
                    case 0xc8: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_UP, false); break;
                    case 0xc9: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_PAGEUP, false); break;
                    case 0xcb: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_LEFT, false); break;
                    case 0xcd: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_RIGHT, false); break;
                    case 0xcf: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_END, false); break;
                    case 0xd0: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_DOWN, false); break;
                    case 0xd1: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_PAGEDOWN, false); break;
                    case 0xd2: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_INSERT, false); break;
                    case 0xd3: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_DELETE, false); break;
                    case 0xdb: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_LLOGO, false); break;
                    case 0xdc: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_RLOGO, false); break;
                    case 0xdd: keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_RMENU, false); break;
                    case 0xb7: driver->buffer[1] = 0xb7; driver->bufferLength = 2; break;
                    default: driver->bufferLength = 0; break;
                }
            } else if(driver->buffer[0] == 0xe1) {
                if(byte == 0x1d) {
                    driver->bufferLength = 2;
                } else {
                    driver->bufferLength = 0;
                }
            }

            break;

        case 2:
            if(driver->buffer[0] == 0xe0) {
                if(byte == 0xe0) {
                    driver->bufferLength = 3;
                } else {
                    driver->bufferLength = 0;
                }
            } else if(driver->buffer[0] == 0xe1) {
                if(byte == 0x45) {
                    driver->bufferLength = 3;
                } else {
                    driver->bufferLength = 0;
                }
            }

            break;

        case 3:
            if(driver->buffer[0] == 0xe0) {
                if(driver->buffer[1] == 0x2a) {
                    if(byte == 0x37) {
                        keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_PRINTSCREEN, true);
                    }
                } else if(driver->buffer[1] == 0xb7) {
                    if(byte == 0xaa) {
                        keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_PRINTSCREEN, false);
                    }
                }

                driver->bufferLength = 0;
            } else if(driver->buffer[0] == 0xe1) {
                if(byte == 0xe1) {
                    driver->bufferLength = 4;
                } else {
                    driver->bufferLength = 0;
                }
            }

            break;

        case 4:
            if(byte == 0x9d) {
                driver->bufferLength = 5;
            } else {
                driver->bufferLength = 0;
            }

            break;

        case 5:
            if(byte == 0xc5) {
                keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_PAUSE, true);
                keyboard_driver_input((keyboard_driver_t *)driver, KEYCODE_PAUSE, false);
            }

            driver->bufferLength = 0;

            break;
    }
}

static void ps2kbd_writeByte(ps2kbd_driver_t *driver, uint8_t byte) {
    driver->lastWrittenByte = byte;
    outb(0x60, byte);
}
