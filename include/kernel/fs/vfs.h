#ifndef __INCLUDE_KERNEL_FS_VFS_H__
#define __INCLUDE_KERNEL_FS_VFS_H__

#include <stddef.h>

#include "kernel/linkedlist.h"
#include "kernel/types.h"

#define C_MAX_FILE_NAME_SIZE 255
#define C_MAX_FS_NAME_SIZE 255

enum te_vfsNodeType {
    E_VFSNODETYPE_FILE,
    E_VFSNODETYPE_DIRECTORY,
    E_VFSNODETYPE_CHARACTER,
    E_VFSNODETYPE_BLOCK,
    E_VFSNODETYPE_MOUNT,
    E_VFSNODETYPE_FIFO,
    E_VFSNODETYPE_LINK
};

struct ts_vfsNode;

struct ts_vfsNodeOperations {
    int (*m_lookup)(
        struct ts_vfsNode *p_node,
        const char *p_name,
        struct ts_vfsNode **p_result
    );
    int (*m_release)(struct ts_vfsNode *p_node);
    int (*m_chmod)(struct ts_vfsNode *p_node, mode_t p_mode);
    int (*m_chown)(struct ts_vfsNode *p_node, uid_t p_uid);
    int (*m_chgrp)(struct ts_vfsNode *p_node, gid_t p_gid);
    int (*m_link)(
        struct ts_vfsNode *p_node,
        struct ts_vfsNode *p_parent,
        const char *p_name
    );
    int (*m_unlink)(struct ts_vfsNode *p_node);
    int (*m_mknod)(
        struct ts_vfsNode *p_node,
        const char *p_name,
        mode_t p_mode,
        dev_t p_device,
        struct ts_vfsNode *p_newNode
    );
};

struct ts_vfsNode {
    enum te_vfsNodeType m_type;
    struct ts_vfsNode *m_parent;
    char m_name[C_MAX_FILE_NAME_SIZE + 1];
    int m_referenceCount;
    // mutex_t m_mutex;
    size_t m_size;
    uid_t m_uid;
    gid_t m_gid;
    mode_t m_mode;
    union {
        dev_t m_device;
        struct ts_linkedList *m_children;
    } m_specific;
    void *m_fileSystemData;
    struct ts_vfsNodeOperations *m_operations;
};

struct ts_vfsFile;

struct ts_vfsFileOperations {
    int (*m_open)(struct ts_vfsFile *p_file, int p_flags);
    int (*m_close)(struct ts_vfsFile *p_file);
    ssize_t (*m_llseek)(
        struct ts_vfsFile *p_file,
        loff_t p_offset,
        int p_whence
    );
    ssize_t (*m_read)(
        struct ts_vfsFile *p_file,
        void *p_buffer,
        size_t p_size,
        loff_t p_offset
    );
    ssize_t (*m_write)(
        struct ts_vfsFile *p_file,
        const void *p_buffer,
        size_t p_size,
        loff_t p_offset
    );
    int (*m_ioctl)(struct ts_vfsFile *p_file, int p_request, void *p_arg);
};

struct ts_vfsFile {
    struct ts_vfsNode *m_node;
    struct ts_vfsFileOperations *m_operations;
    int m_flags;
    loff_t m_offset;
    void *m_fileSystemData;
};

struct ts_vfsFileSystem {
    const char *m_name;
    int (*m_mount)(
        struct ts_vfsFileSystem *p_fileSystem,
        struct ts_vfsNode *p_node
    );
};

int vfsInit(void);
int vfsMount(
    struct ts_vfsNode *p_node,
    struct ts_vfsFileSystem *p_fileSystem
);
int vfsRegisterFileSystem(const struct ts_vfsFileSystem *p_fileSystem);
int vfsGetFileSystem(
    const char *p_name,
    const struct ts_vfsFileSystem **p_fileSystem
);
int vfsUnregisterFileSystem(const struct ts_vfsFileSystem *p_fileSystem);
int vfsLookup(
    struct ts_vfsNode *p_node,
    const char *p_name,
    struct ts_vfsNode **p_result
);
int vfsRelease(struct ts_vfsNode *p_node);
int vfsChmod(struct ts_vfsNode *p_node, mode_t p_mode);
int vfsChown(struct ts_vfsNode *p_node, uid_t p_uid);
int vfsChgrp(struct ts_vfsNode *p_node, gid_t p_gid);
int vfsLink(
    struct ts_vfsNode *p_node,
    struct ts_vfsNode *p_parent,
    const char *p_name,
    struct ts_vfsNode **p_output
);
int vfsUnlink(struct ts_vfsNode *p_node);
int vfsMknod(
    struct ts_vfsNode *p_node,
    const char *p_name,
    mode_t p_mode,
    dev_t p_device,
    struct ts_vfsNode **p_output
);
int vfsOpen(struct ts_vfsNode *p_node, struct ts_vfsFile *p_file, int p_flags);
int vfsClose(struct ts_vfsFile *p_file);
ssize_t vfsLlseek(struct ts_vfsFile *p_file, loff_t p_offset, int p_whence);
ssize_t vfsRead(
    struct ts_vfsFile *p_file,
    void *p_buffer,
    size_t p_size,
    loff_t p_offset
);
ssize_t vfsWrite(
    struct ts_vfsFile *p_file,
    const void *p_buffer,
    size_t p_size,
    loff_t p_offset
);
int vfsIoctl(struct ts_vfsFile *p_file, int p_request, void *p_arg);

#endif
