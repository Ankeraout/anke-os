#include <errno.h>
#include <kernel/debug.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/klibc/string.h>
#include <kernel/misc/spinlock.h>
#include <kernel/misc/tree.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

struct ts_vfsTreeNode {
    char a_name[NAME_MAX + 1];
    struct ts_vfsNode a_file;
};

static int vfsCanonicalizePath(
    const char *p_cwd,
    const char *p_path,
    const char **p_output
);
static int vfsCheckPath(const char *p_path);
static void vfsDisposeNode(struct ts_vfsNode *p_node);

static struct ts_treeNode s_vfsRoot;
static bool s_vfsRootMounted;

int vfsCreateNode(
    struct ts_vfsNode *p_parent,
    const char *p_name,
    bool p_autoFree,
    struct ts_vfsNode **p_output
) {
    // If p_parent is NULL then we consider that the parent is /.
    if(p_parent == NULL) {
        struct ts_vfsTreeNode *l_parentNode = s_vfsRoot.a_data;
        p_parent = &l_parentNode->a_file;
    }

    // Try to allocate memory for the VFS node data
    struct ts_vfsTreeNode *l_node = kcalloc(sizeof(struct ts_vfsTreeNode));

    if(l_node == NULL) {
        return -ENOMEM;
    }

    // Initialize the node
    if(p_autoFree) {
        spinlockInit(&l_node->a_file.a_lock);
        l_node->a_file.a_referenceCount = 1;
    } else {
        l_node->a_file.a_referenceCount = -1;
    }

    strcpy(l_node->a_name, p_name);

    // Add the node to the tree
    l_node->a_file.a_vfs = treeAddChild(p_parent->a_vfs, l_node);

    if(l_node->a_file.a_vfs == NULL) {
        kfree(l_node);
        return -ENOMEM;
    }

    // Return the node
    *p_output = &l_node->a_file;

    return 0;
}

static void vfsDebug2(struct ts_treeNode *p_node, int p_depth) {
    debug("vfs: ");

    for(int l_i = 0; l_i < p_depth; l_i++) {
        debug("    ");
    }

    struct ts_vfsTreeNode *l_node = p_node->a_data;

    debug("- %s\n", l_node->a_name);

    struct ts_linkedListNode *l_childNode = p_node->a_children.a_first;

    while(l_childNode != NULL) {
        vfsDebug2(l_childNode->a_data, p_depth + 1);
        l_childNode = l_childNode->a_next;
    }
}

void vfsDebug(void) {
    debug("vfs: VFS structure:\n");
    vfsDebug2(&s_vfsRoot, 0);
}

int vfsInit(void) {
    // Allocate memory for the root node.
    struct ts_vfsTreeNode *l_vfsTreeNode =
        kcalloc(sizeof(struct ts_vfsTreeNode));

    if(l_vfsTreeNode == NULL) {
        return -ENOMEM;
    }

    // Initialize the tree structure.
    int l_returnValue = treeInit(&s_vfsRoot, l_vfsTreeNode);

    if(l_returnValue != 0) {
        kfree(l_vfsTreeNode);
        return l_returnValue;
    }

    // We give it a special name (but this is not important and will never
    // actually be used in the code).
    strcpy(l_vfsTreeNode->a_name, "/");

    // The root node cannot be disposed.
    l_vfsTreeNode->a_file.a_referenceCount = -1;

    // The root node is always a directory
    l_vfsTreeNode->a_file.a_type = E_VFSNODETYPE_DIRECTORY;

    // All operations are set to NULL by default (thanks to kcalloc). This will
    // remain true until root is mounted.
    s_vfsRootMounted = false;
    l_vfsTreeNode->a_file.a_vfs = &s_vfsRoot;

    // No error code
    return 0;
}

int vfsLookup(
    const char *p_cwd,
    const char *p_path,
    struct ts_vfsNode **p_output
) {
    const char *l_path;
    int l_returnValue = vfsCanonicalizePath(p_cwd, p_path, &l_path);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    // If the path is "/", return the root node.
    if(l_path[1] == '\0') {
        struct ts_vfsTreeNode *l_rootTreeNode = s_vfsRoot.a_data;
        *p_output = &l_rootTreeNode->a_file;
        kfree((void *)l_path);
        return 0;
    }

    // Iterate over path components to find the node.
    struct ts_treeNode *l_currentNode = &s_vfsRoot;
    size_t l_readIndex = 1;

    while(true) {
        struct ts_vfsTreeNode *l_currentNodeData = l_currentNode->a_data;

        // Compute path component
        char l_component[NAME_MAX + 1];
        bool l_isDir = false;
        size_t l_writeIndex = 0;

        while(true) {
            if(l_path[l_readIndex] == '/') {
                l_readIndex++;
                l_isDir = true;
                break;
            } else if(l_path[l_readIndex] == '\0') {
                break;
            } else {
                l_component[l_writeIndex++] = l_path[l_readIndex++];
            }
        }

        l_component[l_writeIndex] = '\0';

        // Try to find a child node with the same name
        struct ts_vfsNode *l_nextNode = NULL;

        struct ts_linkedListNode *l_childNode =
            l_currentNodeData->a_file.a_vfs->a_children.a_first;

        while(l_childNode != NULL) {
            struct ts_treeNode *l_childTreeNode = l_childNode->a_data;
            struct ts_vfsTreeNode *l_childVfsTreeNode = l_childTreeNode->a_data;

            if(strcmp(l_childVfsTreeNode->a_name, l_component) == 0) {
                l_nextNode = &l_childVfsTreeNode->a_file;
                break;
            }
        }

        // If the node was not found then we need to look it up from the
        // current node.
        if(l_nextNode == NULL) {
            l_returnValue = vfsOperationLookup(
                &l_currentNodeData->a_file,
                l_component,
                &l_nextNode
            );

            // If the component could not be found, return the error code.
            if(l_returnValue != 0) {
                kfree((void *)l_path);
                return l_returnValue;
            }
        }

        // Check the node type.
        if(l_isDir && (l_nextNode->a_type != E_VFSNODETYPE_DIRECTORY)) {
            return -ENOTDIR;
        }

        l_currentNode = l_nextNode->a_vfs;

        if(!l_isDir) {
            // If the current component is the last component, we exit the loop.
            break;
        }
    }

    // We do not need the canonicalized path anymore.
    kfree((void *)l_path);

    // Return the found node.
    struct ts_vfsTreeNode *l_currentNodeData = l_currentNode->a_data;
    *p_output = &l_currentNodeData->a_file;

    // No error code.
    return 0;
}

int vfsMount(struct ts_vfsNode *p_node, const struct ts_vfsFileSystem *p_fs) {
    // Make sure that the node is a directory
    if(p_node->a_type != E_VFSNODETYPE_DIRECTORY) {
        return -ENOTDIR;
    }

    // Make sure that the node is not busy and is not already mounted.
    spinlockAcquire(&p_node->a_lock);

    if(p_node->a_referenceCount != 1) {
        // Special case: root node
        if(p_node->a_vfs == &s_vfsRoot) {
            if(s_vfsRootMounted) {
                return -EBUSY;
            }
        } else {
            // The node is busy or already mounted.
            spinlockRelease(&p_node->a_lock);
            return -EBUSY;
        }
    }

    // We set the reference count to -1 so that the VFS never frees this node.
    p_node->a_referenceCount = -1;

    spinlockRelease(&p_node->a_lock);

    int l_returnValue = p_fs->a_onMount(p_node, 0);

    if(l_returnValue != 0) {
        // If an error occurred, restore the reference count.
        p_node->a_referenceCount = 1;
    }

    // Copy the file system operations on the mount node.
    p_node->a_operations = p_fs->a_operations;

    return l_returnValue;
}

int vfsOperationClose(struct ts_vfsNode *p_node) {
    if(p_node == NULL) {
        return -EINVAL;
    }

    // If the node is non-auto-disposable (mount points?) then there is nothing
    // to do here.
    if(p_node->a_referenceCount == -1) {
        return 0;
    }

    spinlockAcquire(&p_node->a_lock);

    p_node->a_referenceCount--;

    if(p_node->a_referenceCount == 0) {
        if(
            (p_node->a_operations != NULL)
            && (p_node->a_operations->a_close != NULL)
        ) {
            p_node->a_operations->a_close(p_node);
        }

        vfsDisposeNode(p_node);
    } else {
        spinlockRelease(&p_node->a_lock);
    }

    return 0;
}

int vfsOperationIoctl(
    struct ts_vfsNode *p_node,
    int p_request,
    void *p_arg
) {
    if(p_node == NULL) {
        return -EINVAL;
    }

    if(p_node->a_operations == NULL) {
        return -EOPNOTSUPP;
    }

    if(p_node->a_operations->a_ioctl == NULL) {
        return -EOPNOTSUPP;
    }

    return p_node->a_operations->a_ioctl(p_node, p_request, p_arg);
}

int vfsOperationLookup(
    struct ts_vfsNode *p_node,
    const char *p_name,
    struct ts_vfsNode **p_output
) {
    if(p_node == NULL) {
        return -EINVAL;
    }

    if(p_node->a_operations == NULL) {
        return -EOPNOTSUPP;
    }

    if(p_node->a_operations->a_lookup == NULL) {
        return -EOPNOTSUPP;
    }

    return p_node->a_operations->a_lookup(p_node, p_name, p_output);
}

int vfsOperationMkdir(
    struct ts_vfsNode *p_node,
    const char *p_name
) {
    if(p_node == NULL) {
        return -EINVAL;
    }

    if(p_node->a_operations == NULL) {
        return -EOPNOTSUPP;
    }

    if(p_node->a_operations->a_mkdir == NULL) {
        return -EOPNOTSUPP;
    }

    // Make sure that the node is a directory.
    if(p_node->a_type != E_VFSNODETYPE_DIRECTORY) {
        return -ENOTDIR;
    }

    return p_node->a_operations->a_mkdir(p_node, p_name);
}

int vfsOperationMknod(
    struct ts_vfsNode *p_node,
    const char *p_name,
    enum te_vfsNodeType p_type,
    dev_t p_deviceNumber
) {
    if(p_node == NULL) {
        return -EINVAL;
    }

    if(p_node->a_operations == NULL) {
        return -EOPNOTSUPP;
    }

    if(p_node->a_operations->a_mknod == NULL) {
        return -EOPNOTSUPP;
    }

    // Make sure that the node is a directory.
    if(p_node->a_type != E_VFSNODETYPE_DIRECTORY) {
        return -ENOTDIR;
    }

    return p_node->a_operations->a_mknod(p_node, p_name, p_type, p_deviceNumber);
}

ssize_t vfsOperationWrite(
    struct ts_vfsNode *p_node,
    const void *p_buffer,
    size_t p_size
) {
    if(p_node == NULL) {
        return -EINVAL;
    }

    if(p_node->a_operations == NULL) {
        return -EOPNOTSUPP;
    }

    if(p_node->a_operations->a_write == NULL) {
        return -EOPNOTSUPP;
    }

    return p_node->a_operations->a_write(p_node, p_buffer, p_size);
}

static int vfsCanonicalizePath(
    const char *p_cwd,
    const char *p_path,
    const char **p_output
) {
    char *l_completePath;
    size_t l_completePathLength;
    int l_returnValue;

    if(p_path[0] == '/') {
        l_completePathLength = strlen(p_path);

        if(l_completePathLength > PATH_MAX) {
            return -ENAMETOOLONG;
        }

        l_completePath = kstrdup(p_path);

        if(l_completePath == NULL) {
            return -ENOMEM;
        }
    } else if((p_cwd == NULL) || (p_cwd[0] != '/')) {
        return -EINVAL;
    } else {
        size_t l_cwdLength = strlen(p_cwd);
        size_t l_pathLength = strlen(p_path);
        l_completePathLength = l_cwdLength + l_pathLength + 1;

        if(l_completePathLength > PATH_MAX) {
            return -ENAMETOOLONG;
        }

        l_completePath = kmalloc(l_completePathLength + 1);

        if(l_completePath == NULL) {
            return -ENOMEM;
        }

        memcpy(l_completePath, p_cwd, l_cwdLength);
        l_completePath[l_cwdLength] = '/';
        memcpy(l_completePath + l_cwdLength + 1, p_path, l_pathLength);
        l_completePath[l_completePathLength] = '\0';
    }

    // Check the validity of the complete path
    l_returnValue = vfsCheckPath(l_completePath);

    if(l_returnValue != 0) {
        kfree(l_completePath);
        return l_returnValue;
    }

    // We start at position 1 because there is '/' at position 0.
    size_t l_readIndex = 1;
    size_t l_writeIndex = 1;

    const char *l_component = &l_completePath[l_readIndex];
    bool l_pathEnd = false;

    while(!l_pathEnd) {
        // Determine component length
        size_t l_componentLength = 0;

        while(true) {
            char l_componentCharacter = l_component[l_componentLength];

            if(l_componentCharacter == '/') {
                break;
            } else if(l_componentCharacter == '\0') {
                l_pathEnd = true;
                break;
            }

            l_completePath[l_writeIndex++] = l_componentCharacter;
            l_componentLength++;
        }

        if(l_componentLength == 0) {
            // If the component is "", ignore it.
        } else if((l_componentLength == 1) && memcmp(l_component, ".", 1) == 0) {
            // If the component is ".", remove it.
            l_writeIndex--;
        } else if((l_componentLength == 2) && (memcmp(l_component, "..", 2) == 0)) {
            // If the component is "..", remove it.
            l_writeIndex -= 2;

            if(l_writeIndex != 1) {
                // If the path is not just "/", then remove the previous
                // component.
                l_completePath[l_writeIndex - 1] = '\0';
                const char *l_lastSeparator = strrchr(l_completePath, '/');
                l_writeIndex = l_lastSeparator - l_completePath + 1;
            }
        } else if(!l_pathEnd) {
            // If the component is a file name, add a '/' after it.
            l_completePath[l_writeIndex++] = '/';
        }

        if(l_pathEnd) {
            // If we reached the end of the path
            if(l_completePath[l_writeIndex - 1] == '/') {
                // If the complete path ends with a '/'
                if(l_writeIndex > 1) {
                    // If the path is not "/", then remove the trailing '/'.
                    l_writeIndex--;
                }
            }

            break;
        } else {
            // Set the pointer to the name of the next component.
            l_component += l_componentLength + 1;
        }
    }

    l_completePath[l_writeIndex] = '\0';
    *p_output = l_completePath;

    return 0;
}

static int vfsCheckPath(const char *p_path) {
    if(p_path[0] != '/') {
        return -EINVAL;
    }

    size_t l_readIndex = 1;
    size_t l_componentLength = 0;

    while(true) {
        if(p_path[l_readIndex] == '/') {
            l_componentLength = 0;
        } else if(p_path[l_readIndex] == '\0') {
            return 0;
        } else if(l_componentLength == NAME_MAX) {
            return -ENAMETOOLONG;
        } else {
            l_componentLength++;
        }

        l_readIndex++;
    }
}

static void vfsDisposeNode(struct ts_vfsNode *p_node) {
    // Get the parent node
    struct ts_treeNode *l_treeNode = p_node->a_vfs->a_parent;

    // If there is no parent, simply return (should only happen with root).
    if(l_treeNode == NULL) {
        return;
    }

    struct ts_vfsTreeNode *l_vfsTreeNode = l_treeNode->a_data;

    // Remove the node from its parent's child list and destroy its children
    // (note that when vfsDisposeNode is called, p_node should have no
    // children).
    if(treeDestroy(p_node->a_vfs) != 0) {
        debug("vfs: Failed to destroy node.\n");
    }

    // Free the node
    kfree(p_node);

    // Call close on the parent
    vfsOperationClose(&l_vfsTreeNode->a_file);
}
