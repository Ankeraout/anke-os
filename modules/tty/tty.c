#include <errno.h>
#include <string.h>

#include <kernel/common.h>
#include <kernel/debug.h>
#include <kernel/dev/device.h>
#include <kernel/module.h>
#include <kernel/fonts/fonts.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <modules/framebuffer.h>
#include <modules/tty.h>

#define C_TTY_DEFAULT_COLOR_BACKGROUND 0
#define C_TTY_DEFAULT_COLOR_FOREGROUND 7

enum te_ttyControlSequenceParserState {
    E_STATE_NORMAL,
    E_STATE_ESC,
    E_STATE_ESC_CSI
};

struct ts_ttyDevice {
    int a_width;
    int a_height;
    int a_x;
    int a_y;
    int a_foregroundColorIndex;
    int a_backgroundColorIndex;
    t_framebufferColor a_foregroundColor;
    t_framebufferColor a_backgroundColor;
    struct ts_vfsNode *a_framebufferDevice;
    struct ts_consoleFont *a_font;
    enum te_ttyControlSequenceParserState a_parserState;
    bool a_controlSequenceValid;
    int a_controlSequenceParameterCount;
    int a_controlSequenceParameterIndex;
    int a_controlSequenceParameters[3];
};

static int ttyInit(const char *p_arg);
static void ttyQuit(void);
static ssize_t ttyWriteDevice(
    struct ts_vfsNode *p_file,
    const void *p_buffer,
    size_t p_size
);
static void ttyWriteByte(struct ts_ttyDevice *p_device, char p_character);
static void ttyWriteByteNormal(struct ts_ttyDevice *p_device, char p_character);
static void ttyCheckSequence(struct ts_ttyDevice *p_device, char p_character);
static void ttyCheckCursorPosition(struct ts_ttyDevice *p_device);
static void ttyHandleSgr(struct ts_ttyDevice *p_device);

static const struct ts_vfsNodeOperations s_ttyOperations = {
    .a_write = ttyWriteDevice
};

M_DECLARE_MODULE struct ts_module g_moduleTty = {
    .a_name = "tty",
    .a_init = ttyInit,
    .a_quit = ttyQuit
};

static const t_framebufferColor s_ttyPalette[] = {
    0xff000000,
    0xff0000aa,
    0xff00aa00,
    0xff00aaaa,
    0xffaa0000,
    0xffaa00aa,
    0xffaaaa00,
    0xffaaaaaa,
    0xff555555,
    0xff5555ff,
    0xff55ff55,
    0xff55ffff,
    0xffff5555,
    0xffff55ff,
    0xffffff55,
    0xffffffff
};

static dev_t s_ttyDeviceNumber;
struct ts_ttyDevice s_ttyDevice;

static int ttyInit(const char *p_arg) {
    M_UNUSED_PARAMETER(p_arg);

    debug("tty: Initializing module...\n");

    s_ttyDeviceNumber = deviceMake(0, 0);

    // Get framebuffer file
    struct ts_vfsNode *l_framebufferDevice;
    int l_returnValue = vfsLookup(NULL, p_arg, &l_framebufferDevice);

    if(l_returnValue != 0) {
        debug("tty: Failed to find %s: %d.\n", p_arg, l_returnValue);
        return l_returnValue;
    }

    // Initialize tty
    s_ttyDevice.a_x = 0;
    s_ttyDevice.a_y = 0;
    s_ttyDevice.a_backgroundColorIndex = C_TTY_DEFAULT_COLOR_BACKGROUND;
    s_ttyDevice.a_foregroundColorIndex = C_TTY_DEFAULT_COLOR_FOREGROUND;
    s_ttyDevice.a_backgroundColor = s_ttyPalette[C_TTY_DEFAULT_COLOR_BACKGROUND];
    s_ttyDevice.a_foregroundColor = s_ttyPalette[C_TTY_DEFAULT_COLOR_FOREGROUND];
    s_ttyDevice.a_framebufferDevice = l_framebufferDevice;
    s_ttyDevice.a_font = &g_font8;
    s_ttyDevice.a_parserState = E_STATE_NORMAL;

    vfsOperationIoctl(
        l_framebufferDevice,
        E_IOCTL_FRAMEBUFFER_GET_WIDTH,
        &s_ttyDevice.a_width
    );
    vfsOperationIoctl(
        l_framebufferDevice,
        E_IOCTL_FRAMEBUFFER_GET_HEIGHT,
        &s_ttyDevice.a_height
    );

    s_ttyDevice.a_width /= 8;
    s_ttyDevice.a_height /= s_ttyDevice.a_font->a_height;

    // Register driver
    l_returnValue =
        deviceRegister(E_DEVICETYPE_CHARACTER, "tty", &s_ttyDeviceNumber, 1);

    if(l_returnValue != 0) {
        debug("tty: Failed to register device.\n");
        return l_returnValue;
    }

    l_returnValue = deviceAdd(
        "tty",
        s_ttyDeviceNumber,
        &s_ttyOperations,
        1
    );

    if(l_returnValue != 0) {
        debug("tty: Failed to add device.\n");
        return l_returnValue;
    }

    l_returnValue = deviceCreateFile(s_ttyDeviceNumber);

    if(l_returnValue != 0) {
        debug("tty: Failed to create device file.\n");
        return l_returnValue;
    }

    debug("tty: Module initialized successfully.\n");

    return 0;
}

static void ttyQuit(void) {

}

static ssize_t ttyWriteDevice(
    struct ts_vfsNode *p_file,
    const void *p_buffer,
    size_t p_size
) {
    M_UNUSED_PARAMETER(p_file);

    const char *l_str = (const char *)p_buffer;

    for(size_t l_index = 0; l_index < p_size; l_index++) {
        ttyWriteByte(&s_ttyDevice, l_str[l_index]);
    }

    return (ssize_t)p_size;
}

static void ttyWriteByte(struct ts_ttyDevice *p_device, char p_character) {
    switch(p_device->a_parserState) {
        case E_STATE_NORMAL:
            if(p_character == '\x1b') {
                p_device->a_parserState = E_STATE_ESC;
                p_device->a_controlSequenceValid = true;
            } else {
                ttyWriteByteNormal(p_device, p_character);
            }

            break;

        case E_STATE_ESC:
            if(p_character == '[') {
                p_device->a_parserState = E_STATE_ESC_CSI;
                p_device->a_controlSequenceParameterCount = 0;
                p_device->a_controlSequenceParameterIndex = 0;
                p_device->a_controlSequenceParameters[0] = 0;
            } else {
                p_device->a_parserState = E_STATE_NORMAL;
            }

            break;

        case E_STATE_ESC_CSI:
            if((p_character >= '0') && (p_character <= '9')) {
                p_device->a_controlSequenceParameters[p_device->a_controlSequenceParameterIndex] *= 10;
                p_device->a_controlSequenceParameters[p_device->a_controlSequenceParameterIndex] += p_character - '0';
                p_device->a_controlSequenceParameterCount = p_device->a_controlSequenceParameterIndex + 1;
            } else if(p_character == ';') {
                if(p_device->a_controlSequenceParameterIndex == 2) {
                    p_device->a_controlSequenceValid = false;
                } else {
                    p_device->a_controlSequenceParameterIndex++;
                    p_device->a_controlSequenceParameters[p_device->a_controlSequenceParameterIndex] = 0;
                }
            } else {
                ttyCheckSequence(p_device, p_character);
            }

            break;
    }
}

static void ttyWriteByteNormal(
    struct ts_ttyDevice *p_device,
    char p_character
) {
    if(p_character == '\0') {
        // Ignore
    } else if(p_character == '\t') {
        p_device->a_x += 4 - (p_device->a_x % 4);
    } else if(p_character == '\n') {
        p_device->a_x = 0;
        p_device->a_y++;
    } else if(p_character == '\r') {
        p_device->a_x = 0;
    } else {
        struct ts_framebufferRequestDrawCharacter l_requestDrawCharacter = {
            .a_backgroundColor = p_device->a_backgroundColor,
            .a_foregroundColor = p_device->a_foregroundColor,
            .a_font = p_device->a_font,
            .a_character = p_character,
            .a_x = p_device->a_x * 8,
            .a_y = p_device->a_y * p_device->a_font->a_height
        };

        vfsOperationIoctl(
            s_ttyDevice.a_framebufferDevice,
            E_IOCTL_FRAMEBUFFER_DRAW_CHARACTER,
            &l_requestDrawCharacter
        );

        p_device->a_x++;
    }

    ttyCheckCursorPosition(p_device);
}

static void ttyCheckSequence(struct ts_ttyDevice *p_device, char p_character) {
    if(!p_device->a_controlSequenceValid) {
        p_device->a_parserState = E_STATE_NORMAL;
        return;
    }

    switch(p_character) {
        case 'A': // Cursor up
            if(p_device->a_controlSequenceParameterCount == 0) {
                p_device->a_controlSequenceParameters[0] = 1;
            }

            p_device->a_y -= p_device->a_controlSequenceParameters[0];

            if(p_device->a_y < 0) {
                p_device->a_y = 0;
            }

            break;

        case 'B': // Cursor down
            if(p_device->a_controlSequenceParameterCount == 0) {
                p_device->a_controlSequenceParameters[0] = 1;
            }

            p_device->a_y += p_device->a_controlSequenceParameters[0];

            if(p_device->a_y >= p_device->a_height) {
                p_device->a_y = p_device->a_height - 1;
            }

            break;

        case 'C': // Cursor forward
            if(p_device->a_controlSequenceParameterCount == 0) {
                p_device->a_controlSequenceParameters[0] = 1;
            }

            p_device->a_x += p_device->a_controlSequenceParameters[0];

            if(p_device->a_x >= p_device->a_width) {
                p_device->a_x = p_device->a_width - 1;
            }

            break;

        case 'D': // Cursor back
            if(p_device->a_controlSequenceParameterCount == 0) {
                p_device->a_controlSequenceParameters[0] = 1;
            }

            p_device->a_x -= p_device->a_controlSequenceParameters[0];

            if(p_device->a_x < 0) {
                p_device->a_x = 0;
            }

            break;

        case 'E': // Cursor next line
            if(p_device->a_controlSequenceParameterCount == 0) {
                p_device->a_controlSequenceParameters[0] = 1;
            }

            p_device->a_x = 0;
            p_device->a_y += p_device->a_controlSequenceParameters[0];

            break;

        case 'F': // Cursor previous line
            if(p_device->a_controlSequenceParameterCount == 0) {
                p_device->a_controlSequenceParameters[0] = 1;
            }

            p_device->a_x = 0;
            p_device->a_y -= p_device->a_controlSequenceParameters[0];

            if(p_device->a_y < 0) {
                p_device->a_y = 0;
            }

            break;

        case 'G': // Cursor horizontal absolute
            if(p_device->a_controlSequenceParameterCount == 0) {
                p_device->a_controlSequenceParameters[0] = 1;
            }

            p_device->a_x = p_device->a_controlSequenceParameters[0] - 1;

            break;

        case 'H': // Cursor position
            if(p_device->a_controlSequenceParameterCount < 2) {
                if(p_device->a_controlSequenceParameterCount == 0) {
                    p_device->a_controlSequenceParameters[0] = 1;
                }

                p_device->a_controlSequenceParameters[1] = 1;
            }

            p_device->a_x = p_device->a_controlSequenceParameters[1] - 1;
            p_device->a_y = p_device->a_controlSequenceParameters[0] - 1;

            break;

        case 'J': // Erase in display
            // TODO: modes 0, 1, 2
            p_device->a_x = 0;
            p_device->a_y = 0;

            {
                struct ts_framebufferRequestFill l_request = {
                    .a_color = p_device->a_backgroundColor
                };

                vfsOperationIoctl(
                    s_ttyDevice.a_framebufferDevice,
                    E_IOCTL_FRAMEBUFFER_FILL,
                    &l_request
                );
            }

            break;

        case 'S':
            if(p_device->a_controlSequenceParameterCount == 0) {
                p_device->a_controlSequenceParameters[0] = 1;
            }

            p_device->a_y = p_device->a_height + p_device->a_controlSequenceParameters[0] - 1;

            break;

        case 'f':
            if(p_device->a_controlSequenceParameterCount < 2) {
                if(p_device->a_controlSequenceParameterCount == 0) {
                    p_device->a_controlSequenceParameters[0] = 1;
                }

                p_device->a_controlSequenceParameters[1] = 1;
            }

            p_device->a_x = p_device->a_controlSequenceParameters[1] - 1;
            p_device->a_y = p_device->a_controlSequenceParameters[0] - 1;

            break;

        case 'm':
            ttyHandleSgr(p_device);
            break;

        default:
            break;
    }

    ttyCheckCursorPosition(p_device);
    p_device->a_parserState = E_STATE_NORMAL;
}

static void ttyCheckCursorPosition(struct ts_ttyDevice *p_device) {
    if(p_device->a_x >= p_device->a_width) {
        p_device->a_x = 0;
        p_device->a_y++;
    }

    if(p_device->a_y >= p_device->a_height) {
        int l_scrollRows = p_device->a_y - p_device->a_height + 1;

        if(l_scrollRows >= p_device->a_height) {
            l_scrollRows = p_device->a_height;
        }

        struct ts_framebufferRequestScrollUp l_requestScrollUp = {
            .a_color = p_device->a_backgroundColor,
            .a_rows = l_scrollRows * p_device->a_font->a_height
        };

        vfsOperationIoctl(
            s_ttyDevice.a_framebufferDevice,
            E_IOCTL_FRAMEBUFFER_SCROLL_UP,
            &l_requestScrollUp
        );

        p_device->a_y -= l_scrollRows;

        if(p_device->a_y < 0) {
            p_device->a_y = 0;
        }
    }
}

static void ttyHandleSgr(struct ts_ttyDevice *p_device) {
    if(p_device->a_controlSequenceParameterCount == 0) {
        p_device->a_controlSequenceParameters[0] = 0;
    }

    const int l_command = p_device->a_controlSequenceParameters[0];

    switch(l_command) {
        case 0: // Reset
            p_device->a_backgroundColorIndex = C_TTY_DEFAULT_COLOR_BACKGROUND;
            p_device->a_foregroundColorIndex = C_TTY_DEFAULT_COLOR_FOREGROUND;
            p_device->a_foregroundColor =
                s_ttyPalette[p_device->a_foregroundColorIndex];
            p_device->a_backgroundColor =
                s_ttyPalette[p_device->a_backgroundColorIndex];
            break;

        case 1:
            p_device->a_foregroundColorIndex |= 0x08;
            p_device->a_foregroundColor =
                s_ttyPalette[p_device->a_foregroundColorIndex];
            break;

        case 2:
            p_device->a_foregroundColorIndex &= 0x07;
            p_device->a_foregroundColor =
                s_ttyPalette[p_device->a_foregroundColorIndex];
            break;

        case 7:
            {
                int l_swap = p_device->a_foregroundColorIndex;
                p_device->a_foregroundColorIndex =
                    p_device->a_backgroundColorIndex;
                p_device->a_backgroundColorIndex = l_swap;
            }

            p_device->a_foregroundColor =
                s_ttyPalette[p_device->a_foregroundColorIndex];
            p_device->a_backgroundColor =
                s_ttyPalette[p_device->a_backgroundColorIndex];

            break;

        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
            p_device->a_foregroundColorIndex = l_command - 30;
            p_device->a_foregroundColor =
                s_ttyPalette[p_device->a_foregroundColorIndex];
            break;

        case 39:
            p_device->a_foregroundColorIndex = C_TTY_DEFAULT_COLOR_FOREGROUND;
            p_device->a_foregroundColor =
                s_ttyPalette[p_device->a_foregroundColorIndex];
            break;

        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
            p_device->a_backgroundColorIndex = l_command - 40;
            p_device->a_backgroundColor =
                s_ttyPalette[p_device->a_backgroundColorIndex];
            break;

        case 49:
            p_device->a_backgroundColorIndex = C_TTY_DEFAULT_COLOR_BACKGROUND;
            p_device->a_backgroundColor =
                s_ttyPalette[p_device->a_backgroundColorIndex];
            break;

        case 90:
        case 91:
        case 92:
        case 93:
        case 94:
        case 95:
        case 96:
        case 97:
            p_device->a_foregroundColorIndex = l_command - 90 + 8;
            p_device->a_foregroundColor =
                s_ttyPalette[p_device->a_foregroundColorIndex];
            break;

        case 100:
        case 101:
        case 102:
        case 103:
        case 104:
        case 105:
        case 106:
        case 107:
            p_device->a_backgroundColorIndex = l_command - 100 + 8;
            p_device->a_backgroundColor =
                s_ttyPalette[p_device->a_backgroundColorIndex];
            break;

        default:
            break;
    }
}
