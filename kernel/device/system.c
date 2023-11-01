#include "common.h"
#include "kernel/device.h"
#include "kernel/device/system.h"
#include "kernel/fs/vfs.h"
#include "klibc/string.h"

#define C_SYSTEM_DEVICE_MAJOR 1
#define C_SYSTEM_DEVICE_MINOR_NULL 3
#define C_SYSTEM_DEVICE_MINOR_ZERO 5

static int deviceSystemOpen(struct ts_vfsFile *p_file, int p_flags);
static int deviceSystemClose(struct ts_vfsFile *p_file);
static ssize_t deviceSystemRead(
    struct ts_vfsFile *p_file,
    void *p_buffer,
    size_t p_size,
    loff_t p_offset
);
static ssize_t deviceSystemWrite(
    struct ts_vfsFile *p_file,
    const void *p_buffer,
    size_t p_size,
    loff_t p_offset
);
static int deviceSystemLlseek(
    struct ts_vfsFile *p_file,
    loff_t p_offset,
    int p_whence
);
static int deviceSystemIoctl(
    struct ts_vfsFile *p_file,
    int p_request,
    void *p_arg
);

static const struct ts_vfsFileOperations s_deviceSystemFileOperations = {
    .m_open = deviceSystemOpen,
    .m_close = deviceSystemClose,
    .m_read = deviceSystemRead,
    .m_write = deviceSystemWrite,
    .m_llseek = deviceSystemLlseek,
    .m_ioctl = deviceSystemIoctl
};

int deviceSystemInit(void) {
    return deviceCharacterRegister(
        C_SYSTEM_DEVICE_MAJOR,
        "system",
        &s_deviceSystemFileOperations
    );
}

static int deviceSystemOpen(struct ts_vfsFile *p_file, int p_flags) {
    M_UNUSED_PARAMETER(p_flags);

    const int l_minor = M_DEVICE_MINOR(p_file->m_vfsNode->m_specific.m_device);

    if(
        (l_minor == C_SYSTEM_DEVICE_MINOR_NULL)
        || (l_minor == C_SYSTEM_DEVICE_MINOR_ZERO)
    ) {
        return 0;
    } else {
        return -ENOTSUP;
    }
}

static int deviceSystemClose(struct ts_vfsFile *p_file) {
    M_UNUSED_PARAMETER(p_file);

    return 0;
}

static ssize_t deviceSystemRead(
    struct ts_vfsFile *p_file,
    void *p_buffer,
    size_t p_size,
    loff_t p_offset
) {
    M_UNUSED_PARAMETER(p_offset);

    const int l_minor = M_DEVICE_MINOR(p_file->m_vfsNode->m_specific.m_device);

    if(l_minor == C_SYSTEM_DEVICE_MINOR_NULL) {
        return 0;
    } else if(l_minor == C_SYSTEM_DEVICE_MINOR_ZERO) {
        kmemset(p_buffer, 0, p_size);
        return p_size;
    } else {
        return -ENOTSUP;
    }
}

static ssize_t deviceSystemWrite(
    struct ts_vfsFile *p_file,
    const void *p_buffer,
    size_t p_size,
    loff_t p_offset
) {
    M_UNUSED_PARAMETER(p_buffer);
    M_UNUSED_PARAMETER(p_offset);

    const int l_minor = M_DEVICE_MINOR(p_file->m_vfsNode->m_specific.m_device);

    if(l_minor == C_SYSTEM_DEVICE_MINOR_NULL) {
        return (ssize_t)p_size;
    } else if(l_minor == C_SYSTEM_DEVICE_MINOR_ZERO) {
        return 0;
    } else {
        return -ENOTSUP;
    }
}

static int deviceSystemLlseek(
    struct ts_vfsFile *p_file,
    loff_t p_offset,
    int p_whence
) {
    M_UNUSED_PARAMETER(p_offset);
    M_UNUSED_PARAMETER(p_whence);

    const int l_minor = M_DEVICE_MINOR(p_file->m_vfsNode->m_specific.m_device);

    if(l_minor == C_SYSTEM_DEVICE_MINOR_NULL) {
        return 0;
    } else if(l_minor == C_SYSTEM_DEVICE_MINOR_ZERO) {
        return 0;
    } else {
        return -ENOTSUP;
    }
}

static int deviceSystemIoctl(
    struct ts_vfsFile *p_file,
    int p_request,
    void *p_arg
) {
    M_UNUSED_PARAMETER(p_request);
    M_UNUSED_PARAMETER(p_arg);

    const int l_minor = M_DEVICE_MINOR(p_file->m_vfsNode->m_specific.m_device);

    if(l_minor == C_SYSTEM_DEVICE_MINOR_NULL) {
        return -ENOTSUP;
    } else if(l_minor == C_SYSTEM_DEVICE_MINOR_ZERO) {
        return -ENOTSUP;
    } else {
        return -ENOTSUP;
    }
}
