#include "kernel/boot.h"
#include "kernel/device.h"
#include "kernel/fs/vfs.h"
#include "klibc/string.h"
#include "sys/stat.h"

#define C_FRAMEBUFFER_DEVICE_MAJOR 2

struct ts_framebufferFileData {
    loff_t m_endOfFileOffset;
};

static const struct ts_bootFramebufferInfo *s_framebufferInfo;

static int framebufferOpen(struct ts_vfsFile *p_file, int p_flags);
static int framebufferClose(struct ts_vfsFile *p_file);
static ssize_t framebufferRead(
    struct ts_vfsFile *p_file,
    void *p_buffer,
    size_t p_size,
    loff_t p_offset
);
static ssize_t framebufferWrite(
    struct ts_vfsFile *p_file,
    const void *p_buffer,
    size_t p_size,
    loff_t p_offset
);
static int framebufferLlseek(
    struct ts_vfsFile *p_file,
    loff_t p_offset,
    int p_whence
);
static int framebufferIoctl(
    struct ts_vfsFile *p_file,
    int p_request,
    void *p_arg
);

static const struct ts_vfsFileOperations s_framebufferFileOperations = {
    .m_open = framebufferOpen,
    .m_close = framebufferClose,
    .m_read = framebufferRead,
    .m_write = framebufferWrite,
    .m_llseek = framebufferLlseek,
    .m_ioctl = framebufferIoctl
};

int framebufferInit(void) {
    const struct ts_kernelBootInfo *l_bootInfo = kernelGetBootInfo();

    if(l_bootInfo->m_framebufferInfo.m_address == NULL) {
        return -ENOSYS;
    }

    s_framebufferInfo = &l_bootInfo->m_framebufferInfo;

    int l_returnValue = deviceCharacterRegister(
        C_FRAMEBUFFER_DEVICE_MAJOR,
        "framebuffer",
        &s_framebufferFileOperations
    );

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    // Create /dev/fb0
    l_returnValue = vfsMknod(
        "/dev/fb0",
        S_IFCHR | 0755,
        M_DEVICE_MAKE(C_FRAMEBUFFER_DEVICE_MAJOR, 0)
    );

    if(l_returnValue != 0) {
        deviceCharacterUnregister(C_FRAMEBUFFER_DEVICE_MAJOR);
        return l_returnValue;
    }

    return 0;
}

static int framebufferOpen(struct ts_vfsFile *p_file, int p_flags) {
    struct ts_framebufferFileData *l_framebufferData =
        kmalloc(sizeof(struct ts_framebufferFileData));

    p_file->m_fileSystemData = l_framebufferData;

    if(p_file->m_fileSystemData == NULL) {
        return -ENOMEM;
    }

    l_framebufferData->m_endOfFileOffset =
        s_framebufferInfo->m_pitch * s_framebufferInfo->m_height;

    if(M_DEVICE_MINOR(p_file->m_vfsNode->m_specific.m_device) != 0) {
        return -EINVAL;
    }

    return 0;
}

static int framebufferClose(struct ts_vfsFile *p_file) {
    kfree(p_file->m_fileSystemData);
    return 0;
}

static ssize_t framebufferRead(
    struct ts_vfsFile *p_file,
    void *p_buffer,
    size_t p_size,
    loff_t p_offset
) {
    struct ts_framebufferFileData *l_framebufferData =
        (struct ts_framebufferFileData *)p_file->m_fileSystemData;

    loff_t l_availableData = l_framebufferData->m_endOfFileOffset
        - p_offset;
    loff_t l_requestedDataLength = (loff_t)p_size;

    if(l_requestedDataLength > l_availableData) {
        l_requestedDataLength = l_availableData;
    }

    if(l_requestedDataLength <= 0) {
        return 0;
    }

    kmemcpy(
        p_buffer,
        (void *)(((size_t)s_framebufferInfo->m_address) + p_offset),
        (size_t)l_requestedDataLength
    );

    return (ssize_t)l_requestedDataLength;
}

static ssize_t framebufferWrite(
    struct ts_vfsFile *p_file,
    const void *p_buffer,
    size_t p_size,
    loff_t p_offset
) {
    struct ts_framebufferFileData *l_framebufferData =
        (struct ts_framebufferFileData *)p_file->m_fileSystemData;

    loff_t l_availableData = l_framebufferData->m_endOfFileOffset - p_offset;
    loff_t l_requestedDataLength = (loff_t)p_size;

    if(l_requestedDataLength > l_availableData) {
        l_requestedDataLength = l_availableData;
    }

    if(l_requestedDataLength <= 0) {
        return 0;
    }

    kmemcpy(
        (void *)(((size_t)s_framebufferInfo->m_address) + p_offset),
        p_buffer,
        (size_t)l_requestedDataLength
    );

    return (ssize_t)l_requestedDataLength;
}

static int framebufferLlseek(
    struct ts_vfsFile *p_file,
    loff_t p_offset,
    int p_whence
) {
    
}

static int framebufferIoctl(
    struct ts_vfsFile *p_file,
    int p_request,
    void *p_arg
) {

}
