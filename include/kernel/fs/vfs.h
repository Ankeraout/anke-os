#ifndef __INCLUDE_KERNEL_FS_VFS_H__
#define __INCLUDE_KERNEL_FS_VFS_H__

#define C_VFS_MAX_FILENAME_SIZE 256

enum te_vfsNodeType {
    E_VFSNODETYPE_FILE,
    E_VFSNODETYPE_FOLDER,
    E_VFSNODETYPE_MOUNTPOINT
};

struct ts_vfsNode {
    enum te_vfsNodeType a_type;
    char a_name[C_VFS_MAX_FILENAME_SIZE];
};

int vfsInit(void);

#endif
