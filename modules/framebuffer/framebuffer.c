#include <errno.h>
#include <stddef.h>
#include <string.h>

#include <kernel/common.h>
#include <kernel/debug.h>
#include <kernel/module.h>
#include <kernel/boot/boot.h>
#include <kernel/fonts/fonts.h>
#include <kernel/fs/devfs.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <modules/framebuffer.h>

struct ts_framebuffer {
    uint32_t *a_buffer;
    size_t a_width;
    size_t a_height;
    size_t a_pitch;
};

static int framebufferInit(const char *p_arg);
static void framebufferQuit(void);
static int framebufferIoctlDriver(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
);
static int framebufferIoctlDevice(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
);
static int framebufferCreate(struct ts_framebufferRequestCreate *p_request);
static void framebufferFill(
    struct ts_framebuffer *p_framebuffer,
    struct ts_framebufferRequestFill *p_request
);
static void framebufferDrawCharacter(
    struct ts_framebuffer *p_framebuffer,
    struct ts_framebufferRequestDrawCharacter *p_request
);
static void framebufferScrollUp(
    struct ts_framebuffer *p_framebuffer,
    struct ts_framebufferRequestScrollUp *p_request
);
static void framebufferSetPixel(
    struct ts_framebuffer *p_framebuffer,
    struct ts_framebufferRequestSetPixel *p_request
);
static inline t_framebufferColor framebufferMergeColors (
    t_framebufferColor l_old,
    t_framebufferColor l_new
);

static int framebufferInit(const char *p_arg) {
    M_UNUSED_PARAMETER(p_arg);

    debug("framebuffer: Initializing module...\n");

    // Create framebuffer device
    struct ts_vfsFileDescriptor *l_framebufferDevice =
        kcalloc(sizeof(struct ts_vfsFileDescriptor));

    if(l_framebufferDevice == NULL) {
        debug("framebuffer: Failed to allocate memory for driver device file.\n");
        return -ENOMEM;
    }

    l_framebufferDevice->a_ioctl = framebufferIoctlDriver;
    strcpy(l_framebufferDevice->a_name, "fb");
    l_framebufferDevice->a_type = E_VFS_FILETYPE_CHARACTER;

    // Register framebuffer device
    struct ts_vfsFileDescriptor *l_dev = vfsOpen("/dev", 0);

    if(l_dev == NULL) {
        debug("framebuffer: Failed to open /dev.\n");

        kfree(l_framebufferDevice);
        // /dev is not mounted?!
        return -ENOENT;
    }

    int l_returnValue = l_dev->a_ioctl(
        l_dev,
        C_IOCTL_DEVFS_CREATE,
        l_framebufferDevice
    );

    kfree(l_dev);

    if(l_returnValue != 0) {
        debug("framebuffer: Failed to create driver device file.\n");
        kfree(l_framebufferDevice);
        return l_returnValue;
    }

    debug("framebuffer: Module initialized successfully.\n");

    return 0;
}

static void framebufferQuit(void) {

}

static int framebufferIoctlDriver(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
) {
    M_UNUSED_PARAMETER(p_file);

    switch(p_request) {
        case E_IOCTL_FRAMEBUFFER_CREATE:
            return framebufferCreate(p_arg);

        default:
            return -EINVAL;
    }
}

static int framebufferIoctlDevice(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
) {
    struct ts_framebuffer *l_framebuffer = p_file->a_context;

    switch(p_request) {
        case E_IOCTL_FRAMEBUFFER_GET_WIDTH:
            return (int)l_framebuffer->a_width;

        case E_IOCTL_FRAMEBUFFER_GET_HEIGHT:
            return (int)l_framebuffer->a_height;

        case E_IOCTL_FRAMEBUFFER_FILL:
            framebufferFill(l_framebuffer, p_arg);
            return 0;

        case E_IOCTL_FRAMEBUFFER_DRAW_CHARACTER:
            framebufferDrawCharacter(l_framebuffer, p_arg);
            return 0;

        case E_IOCTL_FRAMEBUFFER_SCROLL_UP:
            framebufferScrollUp(l_framebuffer, p_arg);
            return 0;

        case E_IOCTL_FRAMEBUFFER_SET_PIXEL:
            framebufferSetPixel(l_framebuffer, p_arg);
            return 0;

        default:
            return -EINVAL;
    }
}

static int framebufferCreate(struct ts_framebufferRequestCreate *p_request) {
    // Get devfs node
    struct ts_vfsFileDescriptor *l_dev = vfsOpen("/dev", 0);

    if(l_dev == NULL) {
        debug("framebuffer: Failed to open /dev.\n");
        return -ENOENT;
    }

    // Allocate memory for framebuffer data
    struct ts_framebuffer *l_framebuffer =
        kmalloc(sizeof(struct ts_framebuffer));

    if(l_framebuffer == NULL) {
        debug("framebuffer: Failed to allocate memory for device data.\n");
        return -ENOMEM;
    }

    // Allocate memory for framebuffer node.
    struct ts_vfsFileDescriptor *l_fb =
        kcalloc(sizeof(struct ts_vfsFileDescriptor));

    if(l_fb == NULL) {
        debug("framebuffer: Failed to allocate memory for device node.\n");
        kfree(l_framebuffer);
        return -ENOMEM;
    }

    // Initialize framebuffer data
    l_framebuffer->a_buffer = p_request->a_buffer;
    l_framebuffer->a_width = p_request->a_width;
    l_framebuffer->a_height = p_request->a_height;
    l_framebuffer->a_pitch = p_request->a_pitch;

    // Initialize framebuffer device node
    strcpy(l_fb->a_name, "fb0");
    l_fb->a_type = E_VFS_FILETYPE_CHARACTER;
    l_fb->a_context = l_framebuffer;
    l_fb->a_ioctl = framebufferIoctlDevice;

    // Register device node
    if(l_dev->a_ioctl(l_dev, C_IOCTL_DEVFS_CREATE, l_fb) != 0) {
        kfree(l_framebuffer);
        kfree(l_fb);
        debug("framebuffer: create: Failed to create /dev/fb0.\n");
        return -EFAULT;
    }

    debug("framebuffer: Registered /dev/fb0.\n");

    return 0;
}

static void framebufferFill(
    struct ts_framebuffer *p_framebuffer,
    struct ts_framebufferRequestFill *p_request
) {
    const size_t l_bufferSize = (p_framebuffer->a_pitch * p_framebuffer->a_width) >> 2;

    for(size_t l_index = 0; l_index < l_bufferSize; l_index++) {
        p_framebuffer->a_buffer[l_index] = framebufferMergeColors(
            p_framebuffer->a_buffer[l_index],
            p_request->a_color
        );
    }
}

static void framebufferDrawCharacter(
    struct ts_framebuffer *p_framebuffer,
    struct ts_framebufferRequestDrawCharacter *p_request
) {
    if(p_request->a_x <= -8) {
        return;
    } else if(p_request->a_x >= (int)p_framebuffer->a_width) {
        return;
    } else if(p_request->a_y <= -8) {
        return;
    } else if(p_request->a_y >= (int)p_framebuffer->a_height) {
        return;
    }

    size_t l_framebufferIndex =
        (p_request->a_y * p_framebuffer->a_pitch >> 2) + p_request->a_x;
    const size_t l_nextRow = (p_framebuffer->a_pitch >> 2) - 8;
    int l_y = p_request->a_y;
    size_t l_fontIndex = p_request->a_character * p_request->a_font->a_height;

    if(p_request->a_y < 0) {
        l_y = 0;
        l_fontIndex -= p_request->a_y;
        l_framebufferIndex += (p_framebuffer->a_pitch >> 2) * -p_request->a_y;
    }

    for(size_t l_row = 0; l_row < (size_t)p_request->a_font->a_height; l_row++) {
        if(l_y >= (int)p_framebuffer->a_height) {
            break;
        }

        int l_x = p_request->a_x;
        uint8_t l_fontData = p_request->a_font->a_data[l_fontIndex++];

        for(size_t l_col = 0; l_col < 8; l_col++) {
            if(l_x < 0) {
                l_x++;
                l_framebufferIndex++;
                continue;
            } else if(l_x >= (int)p_framebuffer->a_width) {
                l_framebufferIndex += 8 - l_col;
                break;
            }

            t_framebufferColor l_color;

            if((l_fontData & 0x80) != 0) {
                l_color = p_request->a_foregroundColor;
            } else {
                l_color = p_request->a_backgroundColor;
            }

            p_framebuffer->a_buffer[l_framebufferIndex] = framebufferMergeColors(
                p_framebuffer->a_buffer[l_framebufferIndex],
                l_color
            );

            l_fontData <<= 1;
            l_x++;
            l_framebufferIndex++;
        }

        l_y++;
        l_framebufferIndex += l_nextRow;
    }
}

static void framebufferScrollUp(
    struct ts_framebuffer *p_framebuffer,
    struct ts_framebufferRequestScrollUp *p_request
) {
    const size_t l_copyRows = p_framebuffer->a_height - p_request->a_rows;
    const size_t l_copySize = l_copyRows * p_framebuffer->a_pitch;
    const size_t l_fillSize = p_request->a_rows * p_framebuffer->a_pitch;
    const size_t l_fillIndex = l_copySize >> 2;
    const size_t l_copyIndex = l_fillSize >> 2;
    const size_t l_fillCount = l_fillSize >> 2;

    memcpy(
        p_framebuffer->a_buffer,
        &p_framebuffer->a_buffer[l_copyIndex],
        l_copySize
    );

    size_t l_index = l_fillIndex;

    for(size_t l_count = 0; l_count < l_fillCount; l_count++) {
        p_framebuffer->a_buffer[l_index++] = p_request->a_color;
    }
}

static void framebufferSetPixel(
    struct ts_framebuffer *p_framebuffer,
    struct ts_framebufferRequestSetPixel *p_request
) {
    if(
        p_request->a_x >= p_framebuffer->a_width
        || p_request->a_y >= p_framebuffer->a_height
    ) {
        return;
    }

    const size_t l_framebufferIndex =
        ((p_request->a_y * p_framebuffer->a_pitch) >> 2) + p_request->a_x;

    p_framebuffer->a_buffer[l_framebufferIndex] = framebufferMergeColors(
        p_framebuffer->a_buffer[l_framebufferIndex],
        p_request->a_color
    );
}

static inline t_framebufferColor framebufferMergeColors (
    t_framebufferColor l_old,
    t_framebufferColor l_new
) {
    uint8_t l_oldRed = l_old >> 16;
    uint8_t l_oldGreen = l_old >> 8;
    uint8_t l_oldBlue = l_old;
    uint8_t l_newRed = l_new >> 16;
    uint8_t l_newGreen = l_new >> 8;
    uint8_t l_newBlue = l_new;
    uint8_t l_newAlpha = l_new >> 24;
    uint8_t l_oldAlpha = ~l_newAlpha;
    uint8_t l_red = (l_oldAlpha * l_oldRed + l_newAlpha * l_newRed) / 255;
    uint8_t l_green = (l_oldAlpha * l_oldGreen + l_newAlpha * l_newGreen) / 255;
    uint8_t l_blue = (l_oldAlpha * l_oldBlue + l_newAlpha * l_newBlue) / 255;

    return (l_red << 16) | (l_green << 8) | l_blue;
}

M_DECLARE_MODULE struct ts_module g_moduleFramebuffer = {
    .a_name = "framebuffer",
    .a_init = framebufferInit,
    .a_quit = framebufferQuit
};
