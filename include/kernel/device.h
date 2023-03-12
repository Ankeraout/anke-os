#ifndef __INCLUDE_KERNEL_DEVICE_H__
#define __INCLUDE_KERNEL_DEVICE_H__

#include <kernel/fs/vfs.h>

struct ts_vfsFileDescriptor *deviceCreate(const char *p_format, int p_first);
int deviceMount(const char *p_path, struct ts_vfsFileDescriptor *p_file);

#endif
