#ifndef __INCLUDE_KERNEL_FS_DEVFS_H__
#define __INCLUDE_KERNEL_FS_DEVFS_H__

#include <kernel/fs/vfs.h>

#define C_IOCTL_DEVFS_CREATE 1

struct ts_vfsFileDescriptor *devfsInit(void);
struct ts_vfsFileDescriptor *devfsCreateDevice(
    struct ts_vfsFileDescriptor *p_devfs,
    const char *p_format,
    int p_firstValue
);

#endif
