#include "kernel/errno.h"
#include "kernel/fs/vfs.h"
#include "kernel/kmalloc.h"
#include "kernel/string.h"

static struct ts_vfsNode s_vfsRootNode = {
    .m_parent = NULL,
    .m_name = "[ROOT]",
    .m_referenceCount = 1,
    .m_size = 0,
    .m_uid = 0,
    .m_gid = 0,
    .m_mode = 0, // TODO
    .m_fileSystemData = NULL,
    .m_operations = NULL
};

static struct ts_linkedList *s_vfsFileSystems = NULL;

int vfsInit(void) {
    
}

int vfsMount(
    struct ts_vfsNode *p_node,
    struct ts_vfsFileSystem *p_fileSystem
) {
    // TODO: checks
    // TODO: lock node
    if(p_node->m_type != E_VFSNODETYPE_DIRECTORY) {
        return -ENOTDIR;
    }

    if(p_node->m_referenceCount != 1) {
        return -EBUSY;
    }

    int l_result = p_fileSystem->m_mount(p_fileSystem, p_node);

    if(l_result != 0) {
        return l_result;
    }

    p_node->m_type = E_VFSNODETYPE_MOUNT;
    p_node->m_referenceCount++;
}

int vfsRegisterFileSystem(const struct ts_vfsFileSystem *p_fileSystem) {
    const int l_result = linkedListAdd(&s_vfsFileSystems, (void *)p_fileSystem);
    int l_returnValue;

    if(l_result != 0) {
        l_returnValue = -ENOMEM;
    } else {
        l_returnValue = 0;
    }

    return l_returnValue;
}

int vfsGetFileSystem(
    const char *p_name,
    const struct ts_vfsFileSystem **p_fileSystem
) {
    struct ts_linkedList *l_element = s_vfsFileSystems;

    while(l_element != NULL) {
        struct ts_vfsFileSystem *l_fileSystem = l_element->m_data;

        if(strncmp(p_name, l_fileSystem->m_name, C_MAX_FS_NAME_SIZE) == 0) {
            *p_fileSystem = l_fileSystem;
            return 0;
        }
        
        l_element = l_element->m_next;
    }

    return -ENODEV;
}

int vfsUnregisterFileSystem(const struct ts_vfsFileSystem *p_fileSystem) {
    const int l_result =
        linkedListRemove(&s_vfsFileSystems, (void *)p_fileSystem);
    int l_returnValue;

    if(l_result == 0) {
        l_returnValue = 0;
    } else {
        l_returnValue = -ENODEV;
    }

    return l_returnValue;
}

int vfsLookup(
    struct ts_vfsNode *p_node,
    const char *p_name,
    struct ts_vfsNode **p_result
) {
    const struct ts_linkedList *l_directoryEntry = 
        p_node->m_specific.m_children;
    struct ts_vfsNode *l_node;

    while(l_directoryEntry != NULL) {
        struct ts_vfsNode *l_node = l_directoryEntry->m_data;

        if(strncmp(l_node->m_name, p_name, C_MAX_FILE_NAME_SIZE) == 0) {
            // TODO: lock
            l_node->m_referenceCount++;
            *p_result = l_node;
            return 0;
        }
        
        l_directoryEntry = l_directoryEntry->m_next;
    }

    // Try to load the node from the file system.
    return p_node->m_operations->m_lookup(p_node, p_name, &l_node);
}

int vfsRelease(struct ts_vfsNode *p_node) {
    // TODO: lock
    p_node->m_referenceCount--;

    if(p_node->m_referenceCount == 0) {
        if(p_node->m_parent != NULL) {
            // TODO: lock parent
            linkedListRemove(&p_node->m_parent->m_specific.m_children, p_node);
            vfsRelease(p_node->m_parent);
        }

        if(p_node->m_operations->m_release != NULL) {
            p_node->m_operations->m_release(p_node);
        }

        kfree(p_node);
    }

    return 0;
}

int vfsChmod(struct ts_vfsNode *p_node, mode_t p_mode) {
    // TODO: check mode validity
    // TODO: lock

    if(p_node->m_operations->m_chmod != NULL) {
        int l_returnValue = p_node->m_operations->m_chmod(p_node, p_mode);

        if(l_returnValue != 0) {
            return l_returnValue;
        }
    }
        
    p_node->m_mode = p_mode;

    return 0;
}

int vfsChown(struct ts_vfsNode *p_node, uid_t p_uid) {
    // TODO: lock
    if(p_node->m_operations->m_chown != NULL) {
        int l_returnValue = p_node->m_operations->m_chown(p_node, p_uid);

        if(l_returnValue != 0) {
            return l_returnValue;
        }
    }
        
    p_node->m_uid = p_uid;

    return 0;
}

int vfsChgrp(struct ts_vfsNode *p_node, gid_t p_gid) {
    // TODO: lock
    if(p_node->m_operations->m_chgrp != NULL) {
        int l_returnValue = p_node->m_operations->m_chgrp(p_node, p_gid);

        if(l_returnValue != 0) {
            return l_returnValue;
        }
    }
        
    p_node->m_gid = p_gid;

    return 0;

}

int vfsLink(
    struct ts_vfsNode *p_node,
    struct ts_vfsNode *p_parent,
    const char *p_name,
    struct ts_vfsNode **p_output
) {
    // TODO: lock
}

int vfsUnlink(struct ts_vfsNode *p_node) {
    // TODO: lock

    if(p_node->m_referenceCount != 1) {
        // TODO: return error (busy)
    }

    if(p_node->m_operations->m_unlink != NULL) {
        int l_returnValue = p_node->m_operations->m_unlink(p_node);

        if(l_returnValue != 0) {
            return l_returnValue;
        }
    }

    vfsRelease(p_node);

    return 0;
}

int vfsMknod(
    struct ts_vfsNode *p_node,
    const char *p_name,
    mode_t p_mode,
    dev_t p_device,
    struct ts_vfsNode **p_output
) {
    // TODO: lock
    if(
        (p_node->m_type != E_VFSNODETYPE_DIRECTORY)
        && (p_node->m_type != E_VFSNODETYPE_MOUNT)
    ) {
        return -ENOTDIR;
    }

    // TODO: validate name

    struct ts_vfsNode *l_node = kmalloc(sizeof(struct ts_vfsNode), 0);

    if(l_node == NULL) {
        return -ENOMEM;
    }

    l_node->m_parent = p_node;
    strncpy(l_node->m_name, p_name, C_MAX_FILE_NAME_SIZE);
    l_node->m_referenceCount = 1;
    l_node->m_size = 0;
    l_node->m_uid = p_node->m_uid;
    l_node->m_gid = p_node->m_gid;
    l_node->m_mode = p_mode;

    // TODO: mode-specific

    if(p_node->m_operations->m_mknod != NULL) {
        int l_returnValue = p_node->m_operations->m_mknod(
            p_node,
            p_name,
            p_mode,
            p_device,
            l_node
        );

        if(l_returnValue != 0) {
            kfree(l_node);
            return l_returnValue;
        }
    }

    linkedListAdd(&p_node->m_specific.m_children, l_node);

    *p_output = l_node;

    return 0;
}

int vfsOpen(struct ts_vfsNode *p_node, struct ts_vfsFile *p_file, int p_flags) {

}

int vfsClose(struct ts_vfsFile *p_file) {

}

ssize_t vfsLlseek(struct ts_vfsFile *p_file, loff_t p_offset, int p_whence) {

}

ssize_t vfsRead(
    struct ts_vfsFile *p_file,
    void *p_buffer,
    size_t p_size,
    loff_t p_offset
) {

}

ssize_t vfsWrite(
    struct ts_vfsFile *p_file,
    const void *p_buffer,
    size_t p_size,
    loff_t p_offset
) {

}

int vfsIoctl(struct ts_vfsFile *p_file, int p_request, void *p_arg) {

}
