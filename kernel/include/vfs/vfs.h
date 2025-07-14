#ifndef __INCLUDE_VFS_VFS_H__
#define __INCLUDE_VFS_VFS_H__

#include "sys/types.h"

#define C_VFS_NAME_LENGTH 31

struct ts_vfs_fileDescriptor;
struct ts_vfs_fileOperations;
struct ts_vfs_fileSystem;
struct ts_vfs_inode;
struct ts_vfs_inodeOperations;
struct ts_vfs_vnode;
struct ts_vfs_vnodeOperations;

struct ts_vfs_fileSystem {
    char m_name[C_VFS_NAME_LENGTH + 1];
    int (*m_mount)(
        struct ts_vfs_fileSystem *p_fileSystem,
        struct ts_vfs_inode *p_deviceNode,
        struct ts_vfs_vnode *p_targetNode,
        int p_flags
    );
};

struct ts_vfs_fileOperations {
    ssize_t (*m_read)(
        struct ts_vfs_fileDescriptor *p_fileDescriptor,
        void *p_buffer,
        size_t p_size
    );
    ssize_t (*m_write)(
        struct ts_vfs_fileDescriptor *p_fileDescriptor,
        const void *p_buffer,
        size_t p_size
    );
    int (*m_close)(struct ts_vfs_fileDescriptor *p_fileDescriptor);
    int (*m_ioctl)(
        struct ts_vfs_fileDescriptor *p_fileDescriptor,
        int p_operation,
        void *p_argument
    );
    off_t (*m_lseek)(
        struct ts_vfs_fileDescriptor,
        off_t p_offset,
        int p_whence
    );
};

struct ts_fileDescriptor {
    struct ts_vfs_inode *m_inode;
    int m_flags;
    struct ts_vfs_fileOperations *m_operations;
    off_t m_offset;
};

struct ts_vfs_inodeOperations {
    int (*m_open)(
        struct ts_vfs_inode *p_inode,
        struct ts_vfs_fileDescriptor *p_fileDescriptor,
        int m_flags
    );
    int (*m_lookup)(
        struct ts_vfs_inode *p_inode,
        const char *p_name,
        struct ts_vfs_vnode **p_vnode
    );
};

struct ts_vfs_inode {
    int m_referenceCount;
    mode_t m_mode;
    gid_t m_gid;
    uid_t m_uid;
    void *m_fileSystemData;
    size_t m_size;
    dev_t m_device;
    struct ts_vfs_inodeOperations *m_operations;
};

struct ts_vfs_vnodeOperations {
    int (*m_unmount)(struct ts_vfs_vnode *p_vnode);
};

struct ts_vfs_vnode {
    char m_name[C_VFS_NAME_LENGTH + 1];
    int m_referenceCount;
    void *m_fileSystemData;
    void *m_treeNode;
    void *m_inode;
    void *m_operations;
};

int vfs_mount(
    struct ts_vfs_inode *p_inode,
    struct ts_vfs_vnode *p_target,
    struct ts_vfs_fileSystem *p_fileSystem,
    int p_flags
);

int vfs_registerFileSystem(struct ts_vfs_fileSystem *p_fileSystem);
struct ts_vfs_fileSystem *vfs_getFileSystemByName(const char *p_name);

#endif
