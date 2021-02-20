#include <stddef.h>
#include <stdint.h>

#include "tty.h"
#include "arch/i686/tty/text16.h"
#include "libk/string.h"

typedef struct {
    tty_api_t api;
    void *buffer;
    int terminalWidth;
    int terminalHeight;
    uint8_t attr;
    int cursorX;
    int cursorY;
    void (*setCursorPosition)(tty_t *tty, int x, int y);
} tty_text16_t;

void tty_text16_init(tty_t *tty, void *buffer, int terminalWidth, int terminalHeight, void (*setCursorPosition)(tty_t *tty, int x, int y));
static int tty_text16_getTerminalWidth(tty_t *tty);
static int tty_text16_getTerminalHeight(tty_t *tty);
static tty_colorMode_t tty_text16_getTerminalColorMode(tty_t *tty);
static void tty_text16_setBackgroundColor(struct tty *tty, int color);
static void tty_text16_setForegroundColor(struct tty *tty, int color);
static void tty_text16_write(struct tty *tty, const char *s);
static void tty_text16_setCursorPosition(tty_t *tty, int x, int y);
static void tty_text16_getCursorPosition(tty_t *tty, int *x, int *y);
static void tty_text16_clear(struct tty *tty);

static tty_api_t tty_text16 = {
    .getTerminalWidth = tty_text16_getTerminalWidth,
    .getTerminalHeight = tty_text16_getTerminalHeight,
    .getTerminalColorMode = tty_text16_getTerminalColorMode,
    .setBackgroundColor = tty_text16_setBackgroundColor,
    .setForegroundColor = tty_text16_setForegroundColor,
    .getCursorPosition = tty_text16_getCursorPosition,
    .setCursorPosition = tty_text16_setCursorPosition,
    .write = tty_text16_write,
    .clear = tty_text16_clear
};

void tty_text16_init(tty_t *tty, void *buffer, int terminalWidth, int terminalHeight, void (*setCursorPosition)(tty_t *tty, int x, int y)) {
    tty_text16_t *tty2 = (tty_text16_t *)tty;
    
    tty2->buffer = buffer;
    tty2->terminalWidth = terminalWidth;
    tty2->terminalHeight = terminalHeight;
    tty2->attr = 0x07;
    tty2->api = tty_text16;
    tty2->setCursorPosition = setCursorPosition;
}

static int tty_text16_getTerminalWidth(tty_t *tty) {
    return ((tty_text16_t *)tty)->terminalWidth;
}

static int tty_text16_getTerminalHeight(tty_t *tty) {
    return ((tty_text16_t *)tty)->terminalHeight;
}

static tty_colorMode_t tty_text16_getTerminalColorMode(tty_t *tty) {
    // Remove compilation warning about unused parameter
    (void)tty;

    return TTY_COLORMODE_4;
}

static void tty_text16_setBackgroundColor(struct tty *tty, int color) {
    ((tty_text16_t *)tty)->attr &= 0x0f;
    ((tty_text16_t *)tty)->attr |= (color << 4) & 0xf0;
}

static void tty_text16_setForegroundColor(struct tty *tty, int color) {
    ((tty_text16_t *)tty)->attr &= 0xf0;
    ((tty_text16_t *)tty)->attr |= color & 0x0f;
}

static inline void tty_text16_checkCursorPosition(tty_text16_t *tty) {
    if(tty->cursorX >= tty->terminalWidth) {
        tty->cursorX = 0;
        tty->cursorY++;
    }

    if(tty->cursorY >= tty->terminalHeight) {
        int lineCount = tty->cursorY - tty->terminalHeight + 1;

        if(lineCount >= tty->terminalHeight - 1) {
            tty->cursorX = 0;
            tty->cursorY = 0;

            for(int i = 0; i < tty->terminalWidth * tty->terminalHeight; i++) {
                ((uint16_t *)tty->buffer)[i] = 0x0700;
            }
        } else {
            size_t offset = tty->terminalWidth * lineCount * 2;
            size_t totalLength = tty->terminalWidth * tty->terminalHeight * 2;
            size_t copyLength = totalLength - offset;

            memcpy(tty->buffer, (uint8_t *)tty->buffer + offset, copyLength);
            memset((uint8_t *)tty->buffer + copyLength, 0, offset);

            tty->cursorY -= lineCount;
        }
    }
}

static void tty_text16_write(struct tty *tty, const char *s) {
    tty_text16_t *tty2 = (tty_text16_t *)tty;
    
    int i = 0;

    while(s[i]) {
        char c = s[i];

        if(c == '\t') {
            tty2->cursorX += 4 - (tty2->cursorX % 4);
        } else if(c == '\n') {
            tty2->cursorX = 0;
            tty2->cursorY++;
        } else if(c == '\r') {
            tty2->cursorX = 0;
        } else {
            int bufferIndex = (tty2->cursorY * tty2->terminalWidth + tty2->cursorX) * 2;
            ((uint8_t *)tty2->buffer)[bufferIndex] = c;
            ((uint8_t *)tty2->buffer)[bufferIndex + 1] = tty2->attr;
            tty2->cursorX++;
        }

        tty_text16_checkCursorPosition(tty2);

        i++;
    }

    tty2->setCursorPosition((tty_t *)tty2, tty2->cursorX, tty2->cursorY);
}

static void tty_text16_setCursorPosition(tty_t *tty, int x, int y) {
    tty_text16_t *tty2 = (tty_text16_t *)tty;

    tty2->cursorX = x;
    tty2->cursorY = y;

    tty_text16_checkCursorPosition(tty2);

    tty2->setCursorPosition((tty_t *)tty2, tty2->cursorX, tty2->cursorY);
}

static void tty_text16_getCursorPosition(tty_t *tty, int *x, int *y) {
    tty_text16_t *tty2 = (tty_text16_t *)tty;

    *x = tty2->cursorX;
    *y = tty2->cursorY;
}

static void tty_text16_clear(tty_t *tty) {
    tty_text16_t *tty2 = (tty_text16_t *)tty;

    tty2->cursorX = 0;
    tty2->cursorY = 0;

    for(int i = 0; i < tty2->terminalWidth * tty2->terminalHeight; i++) {
        ((uint16_t *)tty2->buffer)[i] = tty2->attr << 8;
    }

    tty2->setCursorPosition(tty, 0, 0);
}
