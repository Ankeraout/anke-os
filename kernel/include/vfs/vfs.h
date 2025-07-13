#ifndef __INCLUDE_VFS_VFS_H__
#define __INCLUDE_VFS_VFS_H__

#include "sys/types.h"

#define C_VFS_NAME_LENGTH 31

struct ts_vfsFileDescriptor;
struct ts_vfsFileOperations;
struct ts_vfsFileSystem;
struct ts_vfsInode;
struct ts_vfsInodeOperations;
struct ts_vfsVnode;
struct ts_vfsVnodeOperations;

struct ts_vfsFileSystem {
    char m_name[C_VFS_NAME_LENGTH + 1];
    int (*m_mount)(
        struct ts_vfsFileSystem *p_fileSystem,
        struct ts_vfsInode *p_deviceNode,
        struct ts_vfsVnode *p_targetNode,
        int p_flags
    );
};

struct ts_vfsFileOperations {
    ssize_t (*m_read)(
        struct ts_vfsFileDescriptor *p_fileDescriptor,
        void *p_buffer,
        size_t p_size
    );
    ssize_t (*m_write)(
        struct ts_vfsFileDescriptor *p_fileDescriptor,
        const void *p_buffer,
        size_t p_size
    );
    int (*m_close)(struct ts_vfsFileDescriptor *p_fileDescriptor);
    int (*m_ioctl)(
        struct ts_vfsFileDescriptor *p_fileDescriptor,
        int p_operation,
        void *p_argument
    );
    off_t (*m_lseek)(
        struct ts_vfsFileDescriptor,
        off_t p_offset,
        int p_whence
    );
};

struct ts_fileDescriptor {
    struct ts_vfsInode *m_inode;
    int m_flags;
    struct ts_vfsFileOperations *m_operations;
    off_t m_offset;
};

struct ts_vfsInodeOperations {
    int (*m_open)(
        struct ts_vfsInode *p_inode,
        struct ts_vfsFileDescriptor *p_fileDescriptor,
        int m_flags
    );
    int (*m_lookup)(
        struct ts_vfsInode *p_inode,
        const char *p_name,
        struct ts_vfsVnode **p_vnode
    );
};

struct ts_vfsInode {
    int m_referenceCount;
    mode_t m_mode;
    gid_t m_gid;
    uid_t m_uid;
    void *m_fileSystemData;
    size_t m_size;
    dev_t m_device;
    struct ts_vfsInodeOperations *m_operations;
};

struct ts_vfsVnodeOperations {
    int (*m_unmount)(struct ts_vfsVnode *p_vnode);
};

struct ts_vfsVnode {
    char m_name[C_VFS_NAME_LENGTH + 1];
    int m_referenceCount;
    void *m_fileSystemData;
    void *m_treeNode;
    void *m_inode;
    void *m_operations;
};

int vfs_mount(
    struct ts_vfsInode *p_inode,
    struct ts_vfsVnode *p_target,
    struct ts_vfsFileSystem *p_fileSystem,
    int p_flags
);

int vfs_registerFileSystem(struct ts_vfsFileSystem *p_fileSystem);
struct ts_vfsFileSystem *vfs_getFileSystemByName(const char *p_name);

#endif
