#include <errno.h>
#include <string.h>

#include <kernel/common.h>
#include <kernel/debug.h>
#include <kernel/device.h>
#include <kernel/module.h>
#include <kernel/fonts/fonts.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <modules/framebuffer.h>
#include <modules/tty.h>

struct ts_ttyDevice {
    int a_width;
    int a_height;
    int a_x;
    int a_y;
    t_framebufferColor a_foregroundColor;
    t_framebufferColor a_backgroundColor;
    struct ts_vfsFileDescriptor *a_framebufferDevice;
    struct ts_consoleFont *a_font;
};

static int ttyInit(const char *p_arg);
static void ttyQuit(void);
static int ttyIoctlDriver(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
);
static ssize_t ttyWriteDevice(
    struct ts_vfsFileDescriptor *p_file,
    const void *p_buffer,
    size_t p_size
);
static int ttyCreate(
    struct ts_vfsFileDescriptor *p_driverFile,
    const char *p_framebufferFileName
);

M_DECLARE_MODULE struct ts_module g_moduleTty = {
    .a_name = "tty",
    .a_init = ttyInit,
    .a_quit = ttyQuit
};

static int ttyInit(const char *p_arg) {
    M_UNUSED_PARAMETER(p_arg);

    debug("tty: Initializing module...\n");

    // Create TTY driver file
    struct ts_vfsFileDescriptor *l_ttyDriver =
        kcalloc(sizeof(struct ts_vfsFileDescriptor));

    if(l_ttyDriver == NULL) {
        debug("tty: Failed to allocate memory for driver file.\n");
        return -ENOMEM;
    }

    strcpy(l_ttyDriver->a_name, "tty");
    l_ttyDriver->a_ioctl = ttyIoctlDriver;
    l_ttyDriver->a_type = E_VFS_FILETYPE_CHARACTER;

    if(deviceMount("/dev/%s", l_ttyDriver) != 0) {
        debug("tty: Failed to create driver file.\n");
        kfree(l_ttyDriver);
        return 1;
    }

    debug("tty: Registered /dev/tty.\n");
    debug("tty: Module initialized successfully.\n");

    return 0;
}

static void ttyQuit(void) {

}

static int ttyIoctlDriver(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
) {
    M_UNUSED_PARAMETER(p_file);

    switch(p_request) {
        case E_IOCTL_TTY_CREATE: return ttyCreate(p_file, p_arg);
        default:
            return -EINVAL;
    }

    return -EINVAL;
}

static ssize_t ttyWriteDevice(
    struct ts_vfsFileDescriptor *p_file,
    const void *p_buffer,
    size_t p_size
) {
    struct ts_ttyDevice *l_context = (struct ts_ttyDevice *)p_file->a_context;

    const char *l_str = (const char *)p_buffer;

    for(size_t l_index = 0; l_index < p_size; l_index++) {
        const char l_character = l_str[l_index];

        if(l_character == '\0') {
            // Ignore
            continue;
        } else if(l_character == '\t') {
            l_context->a_x += 4 - (l_context->a_x % 4);
        } else if(l_character == '\n') {
            l_context->a_x = 0;
            l_context->a_y++;
        } else if(l_character == '\r') {
            l_context->a_x = 0;
        } else {
            struct ts_framebufferRequestDrawCharacter l_requestDrawCharacter = {
                .a_backgroundColor = l_context->a_backgroundColor,
                .a_foregroundColor = l_context->a_foregroundColor,
                .a_font = l_context->a_font,
                .a_character = l_character,
                .a_x = l_context->a_x * 8,
                .a_y = l_context->a_y * l_context->a_font->a_height
            };

            l_context->a_framebufferDevice->a_ioctl(
                l_context->a_framebufferDevice,
                E_IOCTL_FRAMEBUFFER_DRAW_CHARACTER,
                &l_requestDrawCharacter
            );

            l_context->a_x++;
        }

        if(l_context->a_x >= l_context->a_width) {
            l_context->a_x = 0;
            l_context->a_y++;
        }

        if(l_context->a_y >= l_context->a_height) {
            int l_scrollRows = l_context->a_y - l_context->a_height + 1;

            if(l_scrollRows >= l_context->a_height) {
                l_scrollRows = l_context->a_height;
            }

            struct ts_framebufferRequestScrollUp l_requestScrollUp = {
                .a_color = l_context->a_backgroundColor,
                .a_rows = l_scrollRows * l_context->a_font->a_height
            };

            l_context->a_framebufferDevice->a_ioctl(
                l_context->a_framebufferDevice,
                E_IOCTL_FRAMEBUFFER_SCROLL_UP,
                &l_requestScrollUp
            );

            l_context->a_y -= l_scrollRows;

            if(l_context->a_y < 0) {
                l_context->a_y = 0;
            }
        }
    }

    return (ssize_t)p_size;
}

static int ttyCreate(
    struct ts_vfsFileDescriptor *p_driverFile,
    const char *p_framebufferFileName
) {
    M_UNUSED_PARAMETER(p_driverFile);

    // Allocate memory for tty context
    struct ts_ttyDevice *l_context = kcalloc(sizeof(struct ts_ttyDevice));

    if(l_context == NULL) {
        debug("tty: Failed to allocate memory for tty context.\n");
        return 1;
    }

    // Open framebuffer driver
    struct ts_vfsFileDescriptor *l_framebufferDevice =
        vfsFind(p_framebufferFileName);

    if(l_framebufferDevice == NULL) {
        debug("tty: Failed to find %s.\n", p_framebufferFileName);
        kfree(l_context);
        return 1;
    }

    l_context->a_foregroundColor = 0xffaaaaaa;
    l_context->a_framebufferDevice = l_framebufferDevice;
    l_context->a_font = &g_font8;

    l_context->a_width = l_framebufferDevice->a_ioctl(
        l_framebufferDevice,
        E_IOCTL_FRAMEBUFFER_GET_WIDTH,
        NULL
    ) / 8;

    l_context->a_height = l_framebufferDevice->a_ioctl(
        l_framebufferDevice,
        E_IOCTL_FRAMEBUFFER_GET_HEIGHT,
        NULL
    ) / l_context->a_font->a_height;

    // Create tty device
    struct ts_vfsFileDescriptor *l_ttyDevice =
        deviceCreate("/dev/tty%d", 0);

    if(l_ttyDevice == NULL) {
        debug("tty: Failed to allocate memory for tty device.\n");
        kfree(l_context);
        kfree(l_framebufferDevice);
        return 1;
    }

    l_ttyDevice->a_type = E_VFS_FILETYPE_CHARACTER;
    l_ttyDevice->a_write = ttyWriteDevice;
    l_ttyDevice->a_context = l_context;

    if(deviceMount("/dev/%s", l_ttyDevice) != 0) {
        debug("tty: Failed to create tty device file.\n");

        kfree(l_context);
        kfree(l_framebufferDevice);
        kfree(l_ttyDevice);

        return 1;
    }

    debug("tty: Registered /dev/%s.\n", l_ttyDevice->a_name);

    return 0;
}
