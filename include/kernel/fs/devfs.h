#ifndef __INCLUDE_KERNEL_FS_DEVFS_H__
#define __INCLUDE_KERNEL_FS_DEVFS_H__

#include <kernel/fs/vfs.h>

#define C_IOCTL_DEVFS_CREATE 1

struct ts_vfsFileDescriptor *devfsInit(void);

#endif
