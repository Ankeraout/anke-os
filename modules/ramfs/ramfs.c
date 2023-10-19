#include "kernel/fs/vfs.h"
#include "kernel/module.h"
#include "klibc/debug.h"
#include "klibc/string.h"
#include "util/list.h"
#include "sys/stat.h"

struct ts_ramfsHeader {
    char m_name[C_MAX_FILE_NAME_SIZE];
    enum te_vfsNodeType m_type;
    int m_uid;
    int m_gid;
    int m_mod;
};

struct ts_ramfsDirectory {
    struct ts_ramfsHeader m_header;
    struct ts_list *m_fileList;
};

struct ts_ramfsFile {
    struct ts_ramfsHeader m_header;
    dev_t m_device;
    size_t m_size;
};

static int ramfsInit(void);
static void ramfsExit(void);
static int ramfsMount(
    struct ts_vfsNode *p_mountNode,
    const struct ts_vfsMountParameters *p_mountParameters
);
static int ramfsNodeDirectoryLookup(
    struct ts_vfsNode *p_node,
    const char *p_name,
    struct ts_vfsNode **p_result
);
static int ramfsNodeRelease(struct ts_vfsNode *p_node);
static int ramfsNodeDirectoryMknod(
    struct ts_vfsNode *p_node,
    const char *p_name,
    mode_t p_mode,
    dev_t p_device
);
static int ramfsNodeChmod(struct ts_vfsNode *p_node, mode_t p_mode);
static int ramfsNodeChown(struct ts_vfsNode *p_node, uid_t p_owner);
static int ramfsNodeChgrp(struct ts_vfsNode *p_node, gid_t p_group);
static int ramfsNodeFileOpen(
    struct ts_vfsNode *p_node,
    int p_flags,
    struct ts_vfsFile *p_file
);

static const struct ts_vfsFileSystem s_ramfsFileSystem = {
    .m_name = "ramfs",
    .m_mount = ramfsMount
};

static const struct ts_vfsNodeOperations s_ramfsNodeOperationsDirectory = {
    .m_lookup = ramfsNodeDirectoryLookup,
    .m_release = ramfsNodeRelease,
    .m_open = NULL,
    .m_chmod = ramfsNodeChmod,
    .m_chown = ramfsNodeChown,
    .m_chgrp = ramfsNodeChgrp,
    .m_link = NULL,
    .m_unlink = NULL,
    .m_mknod = ramfsNodeDirectoryMknod
};

static const struct ts_vfsNodeOperations s_ramfsNodeOperationsFile = {
    .m_lookup = NULL,
    .m_release = ramfsNodeRelease,
    .m_open = ramfsNodeFileOpen,
    .m_chmod = ramfsNodeChmod,
    .m_chown = ramfsNodeChown,
    .m_chgrp = ramfsNodeChgrp,
    .m_link = NULL,
    .m_unlink = NULL,
    .m_mknod = NULL
};

static const struct ts_vfsFileOperations s_ramfsFileOperationsFile = {
    .m_open = NULL,
    .m_read = NULL,
    .m_write = NULL,
    .m_close = NULL,
    .m_llseek = NULL,
    .m_ioctl = NULL
};

static int ramfsMount(
    struct ts_vfsNode *p_mountNode,
    const struct ts_vfsMountParameters *p_mountParameters
) {
    p_mountNode->m_fileSystemData = kmalloc(sizeof(struct ts_ramfsDirectory));

    if(p_mountNode->m_fileSystemData == NULL) {
        return -ENOMEM;
    }

    struct ts_ramfsDirectory *l_directory = p_mountNode->m_fileSystemData;

    l_directory->m_header.m_name[0] = '\0';
    l_directory->m_header.m_type = E_VFSNODETYPE_DIRECTORY;
    l_directory->m_header.m_uid = 0;
    l_directory->m_header.m_gid = 0;
    l_directory->m_header.m_mod = 0755;
    l_directory->m_fileList = NULL;

    p_mountNode->m_operations = &s_ramfsNodeOperationsDirectory;

    return 0;
}

static int ramfsInit(void) {
    kernelDebug("ramfs: Registering ramfs...\n");
    
    int l_returnValue = vfsRegisterFileSystem(&s_ramfsFileSystem);

    if(l_returnValue != 0) {
        kernelDebug(
            "ramfs: Error while registering ramfs file system: code %d.\n",
            l_returnValue
        );

        return l_returnValue;
    }
        
    kernelDebug("ramfs: ramfs file system registered successfully.\n");

    return 0;
}

static void ramfsExit(void) {
    kernelDebug("ramfs: Unregistering ramfs...\n");
    
    int l_returnValue = vfsUnregisterFileSystem(&s_ramfsFileSystem);

    if(l_returnValue != 0) {
        kernelDebug(
            "ramfs: Error while unregistering ramfs file system: code %d.\n",
            l_returnValue
        );
    } else {
        kernelDebug("ramfs: ramfs file system unregistered successfully.\n");
    }
}

static int ramfsNodeDirectoryLookup(
    struct ts_vfsNode *p_node,
    const char *p_name,
    struct ts_vfsNode **p_result
) {
    kernelDebug("ramfs: lookup \"%s\"\n", p_name);

    struct ts_ramfsDirectory *l_directory = p_node->m_fileSystemData;
    struct ts_vfsNode *l_node;

    // Locate the file
    struct ts_ramfsHeader *l_header = NULL;

    {
        struct ts_list *l_node = l_directory->m_fileList;

        while(l_node != NULL) {
            l_header = l_node->m_data;

            if(kstrcmp(l_header->m_name, p_name) == 0) {
                break;
            }

            l_node = l_node->m_next;
        }

        if(l_node == NULL) {
            return -ENOENT;
        }
    }

    // Create the VFS node
    l_node = kmalloc(sizeof(struct ts_vfsNode));

    if(l_node == NULL) {
        return -ENOMEM;
    }

    kstrncpy(l_node->m_name, l_header->m_name, C_MAX_FILE_NAME_SIZE);
    l_node->m_type = l_header->m_type;
    l_node->m_uid = l_header->m_uid;
    l_node->m_gid = l_header->m_gid;
    l_node->m_mod = l_header->m_mod;
    l_node->m_fileSystemData = l_header;

    if(l_header->m_type == E_VFSNODETYPE_DIRECTORY) {
        l_node->m_size = 0;
        l_node->m_operations = &s_ramfsNodeOperationsDirectory;
    } else {
        struct ts_ramfsFile *l_file = (struct ts_ramfsFile *)l_header;

        l_node->m_size = l_file->m_size;
        l_node->m_specific.m_device = l_file->m_device;
        l_node->m_operations = &s_ramfsNodeOperationsFile;
    }

    *p_result = l_node;

    return 0;
}

static int ramfsNodeChmod(struct ts_vfsNode *p_node, mode_t p_mode) {
    struct ts_ramfsHeader *l_header =
        (struct ts_ramfsHeader *)p_node->m_fileSystemData;

    l_header->m_mod &= ~0777;
    l_header->m_mod |= p_mode & 0777;

    return 0;
}

static int ramfsNodeChown(struct ts_vfsNode *p_node, uid_t p_owner) {
    struct ts_ramfsHeader *l_header =
        (struct ts_ramfsHeader *)p_node->m_fileSystemData;
    
    l_header->m_uid = p_owner;

    return 0;
}

static int ramfsNodeChgrp(struct ts_vfsNode *p_node, gid_t p_group) {
    struct ts_ramfsHeader *l_header =
        (struct ts_ramfsHeader *)p_node->m_fileSystemData;

    l_header->m_gid = p_group;

    return 0;
}

static int ramfsNodeRelease(struct ts_vfsNode *p_node) {
    return 0;
}

static int ramfsNodeDirectoryMknod(
    struct ts_vfsNode *p_node,
    const char *p_name,
    mode_t p_mode,
    dev_t p_device
) {
    // Determine file type
    enum te_vfsNodeType l_type;

    if(S_ISREG(p_mode)) {
        l_type = E_VFSNODETYPE_FILE;
    } else if(S_ISDIR(p_mode)) {
        l_type = E_VFSNODETYPE_DIRECTORY;
    } else if(S_ISBLK(p_mode)) {
        l_type = E_VFSNODETYPE_BLOCK;
    } else if(S_ISCHR(p_mode)) {
        l_type = E_VFSNODETYPE_CHAR;
    } else {
        return -EINVAL;
    }

    struct ts_ramfsDirectory *l_directory = p_node->m_fileSystemData;

    // Check if there is already a file with the same name
    {
        struct ts_list *l_element = l_directory->m_fileList;

        while(l_element != NULL) {
            struct ts_ramfsHeader *l_header = l_element->m_data;

            if(kstrncmp(l_header->m_name, p_name, C_MAX_FILE_NAME_SIZE) == 0) {
                return -EEXIST;
            }

            l_element = l_element->m_next;
        }
    }

    // Create the new file
    size_t l_structureSize;

    if(l_type == E_VFSNODETYPE_DIRECTORY) {
        l_structureSize = sizeof(struct ts_ramfsDirectory);
    } else {
        l_structureSize = sizeof(struct ts_ramfsFile);
    }

    struct ts_ramfsHeader *l_header = kmalloc(l_structureSize);

    if(l_header == NULL) {
        return -ENOMEM;
    }

    if(l_type == E_VFSNODETYPE_DIRECTORY) {
        struct ts_ramfsDirectory *l_directory =
            (struct ts_ramfsDirectory *)l_header;

        l_directory->m_fileList = NULL;
    } else {
        struct ts_ramfsFile *l_file =
            (struct ts_ramfsFile *)l_header;

        l_file->m_device = p_device;
        l_file->m_size = 0;
    }

    kstrncpy(l_header->m_name, p_name, C_MAX_FILE_NAME_SIZE);
    l_header->m_type = l_type;
    l_header->m_uid = p_node->m_uid;
    l_header->m_gid = p_node->m_gid;
    l_header->m_mod = p_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

    // Register the new file as child of the parent
    int l_returnValue = listAdd(&l_directory->m_fileList, l_header);

    if(l_returnValue != 0) {
        kfree(l_header);
        return l_returnValue;
    }

    return 0;
}

static int ramfsNodeFileOpen(
    struct ts_vfsNode *p_node,
    int p_flags,
    struct ts_vfsFile *p_file
) {
    p_file->m_operations = &s_ramfsFileOperationsFile;

    return 0;
}

M_DECLARE_MODULE("ramfs", ramfsInit, ramfsExit);
