#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/debug.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/klibc/string.h>
#include <kernel/misc/tree.h>

#define C_VFS_PATH_SEPARATOR '/'

static struct ts_treeNode s_vfsRootNode;
static t_spinlock s_vfsSpinlock;

static void vfsDebug2(struct ts_treeNode *p_node, size_t p_depth);
static void vfsGetMountPoint2(
    const char *p_path,
    const char **p_relativePath,
    struct ts_treeNode *p_node,
    struct ts_vfsFileDescriptor **p_returnValue
);

void vfsDebug(void) {
    debug("vfs: Structure:\n");
    vfsDebug2(&s_vfsRootNode, 0);
}

int vfsInit(void) {
    struct ts_vfsNode *l_rootNode =
        kmalloc(sizeof(struct ts_vfsNode));

    if(l_rootNode == NULL) {
        debug("vfs: Failed to allocate memory for root node information.\n");
        return 1;
    }

    l_rootNode->a_name = "[ROOT]";

    if(treeInit(&s_vfsRootNode, l_rootNode) != 0) {
        debug("vfs: Failed to allocate memory for root node.\n");
        kfree(l_rootNode);
        return 1;
    }

    spinlockInit(&s_vfsSpinlock);

    debug("vfs: VFS initialized.\n");

    return 0;
}

struct ts_vfsFileDescriptor *vfsGetMountPoint(
    const char *p_path,
    const char **p_relativePath
) {
    if(p_path[0] != C_VFS_PATH_SEPARATOR) {
        debug("vfs: getMountPoint: invalid path.\n");
        return NULL;
    }

    struct ts_vfsFileDescriptor *l_returnValue = NULL;

    spinlockAcquire(&s_vfsSpinlock);

    // Check if root is mounted
    struct ts_vfsNode *l_rootVfsNode = s_vfsRootNode.a_data;

    if(l_rootVfsNode->a_fileDescriptor != NULL) {
        l_returnValue = l_rootVfsNode->a_fileDescriptor;
        *p_relativePath = &p_path[1];
    }

    vfsGetMountPoint2(
        &p_path[1],
        p_relativePath,
        &s_vfsRootNode,
        &l_returnValue
    );

    spinlockRelease(&s_vfsSpinlock);

    return l_returnValue;
}

int vfsMount(
    const char *p_mountPath,
    struct ts_vfsFileDescriptor *p_fileDescriptor
) {
    if(p_mountPath[0] != C_VFS_PATH_SEPARATOR) {
        debug("vfs: mount: invalid path (does not start with /).\n");
        return -EINVAL;
    }

    if(p_fileDescriptor == NULL) {
        debug("vfs: mount: cannot mount NULL file descriptor.\n");
        return -EINVAL;
    }

    size_t l_pathIndex = 1;

    spinlockAcquire(&s_vfsSpinlock);

    struct ts_treeNode *l_currentNode = &s_vfsRootNode;
    struct ts_vfsNode *l_currentNodeVfs = l_currentNode->a_data;

    while(true) {
        char l_fileNameBuffer[NAME_MAX + 1];
        size_t l_fileNameLength = 0;
        bool l_isValid = true;
        bool l_isDirectory = false;

        while(l_isValid) {
            char l_fileNameCharacter = p_mountPath[l_pathIndex++];

            if(l_fileNameCharacter == C_VFS_PATH_SEPARATOR) {
                l_isDirectory = true;
                break;
            } else if(l_fileNameCharacter == '\0') {
                break;
            } else if(l_fileNameLength == NAME_MAX) {
                l_isValid = false;
                break;
            } else {
                l_fileNameBuffer[l_fileNameLength++] = l_fileNameCharacter;
            }
        }

        l_fileNameBuffer[l_fileNameLength] = '\0';

        if(!l_isValid) {
            spinlockRelease(&s_vfsSpinlock);
            debug("vfs: mount: Invalid mount path (file name too long).\n");
            return -EINVAL;
        }

        // Look for a child with this name.
        if(l_fileNameBuffer[0] == '\0') {
            // Mount here
            break;
        }

        struct ts_linkedListNode *l_child = l_currentNode->a_children.a_first;
        struct ts_treeNode *l_childNode = NULL;
        struct ts_vfsNode *l_childNodeVfs;

        while(l_child != NULL) {
            l_childNode = l_child->a_data;
            l_childNodeVfs = l_childNode->a_data;

            if(strcmp(l_childNodeVfs->a_name, l_fileNameBuffer) == 0) {
                break;
            }

            l_child = l_child->a_next;
        }

        // Child not found, we create it
        if(l_child == NULL) {
            l_childNodeVfs = kmalloc(sizeof(struct ts_vfsNode));

            if(l_childNodeVfs == NULL) {
                debug("vfs: mount: Failed to allocate memory.\n");
                kfree(l_childNode);
                spinlockRelease(&s_vfsSpinlock);
                return -ENOMEM;
            }

            l_childNodeVfs->a_name = kstrdup(l_fileNameBuffer);

            if(l_childNodeVfs->a_name == NULL) {
                debug("vfs: mount: Failed to allocate memory.\n");
                kfree(l_childNodeVfs);
                kfree(l_childNode);
                spinlockRelease(&s_vfsSpinlock);
                return -ENOMEM;
            }

            l_childNodeVfs->a_fileDescriptor = NULL;

            l_childNode = treeAddChild(l_currentNode, l_childNodeVfs);

            if(l_childNode == NULL) {
                debug("vfs: mount: Failed to allocate memory.\n");
                kfree(l_childNodeVfs->a_name);
                kfree(l_childNodeVfs);
                kfree(l_childNode);
                spinlockRelease(&s_vfsSpinlock);
                return -ENOMEM;
            }
        }

        l_currentNode = l_childNode;

        // If the mount path ends here, we have found the node.
        if(!l_isDirectory) {
            break;
        }
    }

    // Mount
    l_currentNodeVfs = l_currentNode->a_data;

    if(l_currentNodeVfs->a_fileDescriptor != NULL) {
        debug("vfs: mount: %s was already mounted!\n", p_mountPath);
    }

    l_currentNodeVfs->a_fileDescriptor = p_fileDescriptor;

    spinlockRelease(&s_vfsSpinlock);

    debug("vfs: mounted %s.\n", p_mountPath);

    return 0;
}

struct ts_vfsFileDescriptor *vfsFind(const char *p_path) {
    const char *l_relativePath = NULL;
    struct ts_vfsFileDescriptor *l_currentDescriptor =
        vfsGetMountPoint(p_path, &l_relativePath);

    if(l_relativePath[0] == '\0') {
        return l_currentDescriptor;
    }

    while(true) {
        if(l_currentDescriptor == NULL) {
            return NULL;
        }

        if(l_currentDescriptor->a_operations->a_find == NULL) {
            kfree(l_currentDescriptor);
            return NULL;
        }

        // Extract next file name
        const char *l_fileNameEnd =
            strchr(l_relativePath, C_VFS_PATH_SEPARATOR);
        int l_fileNameLength;

        if(l_fileNameEnd == NULL) {
            l_fileNameLength = strlen(l_relativePath);
        } else {
            l_fileNameLength = l_fileNameEnd - l_relativePath;
        }

        if(strlen(l_relativePath) > NAME_MAX) {
            kfree(l_currentDescriptor);
            return NULL;
        }

        char l_fileNameBuffer[NAME_MAX + 1];

        strncpy(l_fileNameBuffer, l_relativePath, l_fileNameLength);

        // Find descriptor
        struct ts_vfsFileDescriptor *l_nextDescriptor =
            l_currentDescriptor->a_operations->a_find(
                l_currentDescriptor,
                l_fileNameBuffer
            );

        kfree(l_currentDescriptor);

        if(l_fileNameEnd == NULL) {
            return l_currentDescriptor;
        } else {
            l_currentDescriptor = l_nextDescriptor;
            l_relativePath += l_fileNameLength + 1;
        }
    }
}

struct ts_vfsFileDescriptor *vfsClone(struct ts_vfsFileDescriptor *p_file) {
    struct ts_vfsFileDescriptor *l_clone =
        kmalloc(sizeof(struct ts_vfsFileDescriptor));

    if(l_clone == NULL) {
        debug("vfs: clone: Failed to allocate memory.\n");
        return NULL;
    }

    memcpy(l_clone, p_file, sizeof(struct ts_vfsFileDescriptor));

    return l_clone;
}

int vfsOpen(struct ts_vfsFileDescriptor *p_file, int p_flags) {
    if(p_file->a_referenceCount >= 0) {
        spinlockAcquire(&p_file->a_referenceCountLock);

        p_file->a_referenceCount++;

        spinlockRelease(&p_file->a_referenceCountLock);
    }

    if(p_file->a_operations->a_open != NULL) {
        if(p_file->a_operations->a_open(p_file, p_flags) != 0) {
            vfsClose(p_file);
            return 1;
        }
    }

    return 0;
}

void vfsClose(struct ts_vfsFileDescriptor *p_file) {
    if(p_file->a_referenceCount < 0) {
        return;
    }

    spinlockAcquire(&p_file->a_referenceCountLock);

    p_file->a_referenceCount--;

    if(p_file->a_referenceCount == 0) {
        if(p_file->a_operations->a_close != NULL) {
            p_file->a_operations->a_close(p_file);
        }

        kfree(p_file);
    } else {
        spinlockRelease(&p_file->a_referenceCountLock);
    }
}

struct ts_vfsFileDescriptor *vfsCreateDescriptor(bool p_disposable) {
    struct ts_vfsFileDescriptor *l_descriptor =
        kcalloc(sizeof(struct ts_vfsFileDescriptor));

    if(l_descriptor == NULL) {
        return NULL;
    }

    spinlockInit(&l_descriptor->a_referenceCountLock);

    if(!p_disposable) {
        l_descriptor->a_referenceCount = -1;
    }

    return l_descriptor;
}

static void vfsDebug2(
    struct ts_treeNode *p_node,
    size_t p_depth
) {
    debug("vfs: ");

    for(size_t l_depth = 0; l_depth < p_depth; l_depth++) {
        debug("    ");
    }

    debug("- %s\n", ((struct ts_vfsNode *)p_node->a_data)->a_name);

    struct ts_linkedListNode *l_child = p_node->a_children.a_first;

    while(l_child != NULL) {
        vfsDebug2(l_child->a_data, p_depth + 1);
        l_child = l_child->a_next;
    }
}

static void vfsGetMountPoint2(
    const char *p_path,
    const char **p_relativePath,
    struct ts_treeNode *p_node,
    struct ts_vfsFileDescriptor **p_returnValue
) {
    size_t l_pathIndex = 0;
    bool l_isValid = true;
    bool l_isFile = true;
    char l_fileName[NAME_MAX + 1];
    size_t l_fileNameLength = 0;

    // Compute file name
    while(true) {
        char l_fileNameCharacter = p_path[l_pathIndex];

        if(l_fileNameCharacter == C_VFS_PATH_SEPARATOR) {
            l_isFile = false;
            break;
        } else if(l_fileNameCharacter == '\0') {
            break;
        } else if(l_fileNameLength == NAME_MAX) {
            l_isValid = false;
            break;
        } else {
            l_fileName[l_fileNameLength++] = l_fileNameCharacter;
            l_pathIndex++;
        }
    }

    l_fileName[l_fileNameLength] = '\0';

    if(!l_isValid) {
        debug("vfs: getMountPoint: invalid path.\n");
        *p_returnValue = NULL;
        return;
    }

    // Try to find a node with this name
    struct ts_linkedListNode *l_childListNode = p_node->a_children.a_first;
    struct ts_treeNode *l_childTreeNode;
    struct ts_vfsNode *l_childVfsNode;

    while(l_childListNode != NULL) {
        l_childTreeNode = l_childListNode->a_data;
        l_childVfsNode = l_childTreeNode->a_data;

        if(strcmp(l_childVfsNode->a_name, l_fileName) == 0) {
            break;
        }

        l_childListNode = l_childListNode->a_next;
    }

    if(l_childListNode == NULL) {
        // We cannot find a mount point any further.
        return;
    }

    struct ts_vfsFileDescriptor *l_fileDescriptor =
        l_childVfsNode->a_fileDescriptor;

    // Check if the current node is a mount point
    const char *l_relativePath;

    if(l_isFile) {
        l_relativePath = &p_path[l_pathIndex];
    } else {
        l_relativePath = &p_path[l_pathIndex + 1];
    }

    if(l_fileDescriptor != NULL) {
        *p_relativePath = l_relativePath;
        *p_returnValue = l_fileDescriptor;
    }

    // Check if we can find a mount point further
    if(!l_isFile) {
        vfsGetMountPoint2(
            l_relativePath,
            p_relativePath,
            l_childTreeNode,
            p_returnValue
        );
    }
}
