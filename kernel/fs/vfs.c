#include <stdbool.h>

#include "kernel/fs/vfs.h"

#include "klibc/debug.h"
#include "klibc/limits.h"
#include "klibc/string.h"
#include "util/list.h"

static struct ts_vfsNode s_vfsRootNode;
static struct ts_list *s_vfsFileSystemList;

static int vfsCanonicalizePath(const char *p_path, char *p_buffer);
static bool vfsIsPathValid(const char *p_path);

int vfsInit(void) {
    s_vfsRootNode.m_referenceCount = -1;
    s_vfsRootNode.m_parent = NULL;
    kstrcpy(s_vfsRootNode.m_name, "[ROOT]");
    s_vfsRootNode.m_type = E_VFSNODETYPE_DIRECTORY;
    s_vfsRootNode.m_specific.m_children = NULL;
    s_vfsRootNode.m_uid = 0;
    s_vfsRootNode.m_gid = 0;
    s_vfsRootNode.m_mod = 0755;
    s_vfsRootNode.m_operations = NULL;

    s_vfsFileSystemList = NULL;

    kernelDebug("vfs: VFS initialized.\n");

    return 0;
}

int vfsLookup(const char *p_path, struct ts_vfsNode **p_result) {
    char l_canonicalizedPath[PATH_MAX + 1];

    int l_returnValue = vfsCanonicalizePath(p_path, l_canonicalizedPath);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    int l_index = 1;
    int l_nameStartIndex = 1;
    struct ts_vfsNode *l_currentNode = &s_vfsRootNode;

    // Special case: root directory
    if(p_path[1] == '\0') {
        *p_result = &s_vfsRootNode;
        return 0;
    }

    while(true) {
        if(p_path[l_index] == '/' || p_path[l_index] == '\0') {
            const int l_nameLength = l_index - l_nameStartIndex;

            char l_name[l_nameLength + 1];

            kmemcpy(l_name, &p_path[l_nameStartIndex], l_nameLength);
            l_name[l_nameLength] = '\0';

            // Check if the next node is a directory
            if(
                l_currentNode->m_type != E_VFSNODETYPE_DIRECTORY
                && l_currentNode->m_type != E_VFSNODETYPE_MOUNT
            ) {
                vfsRelease(l_currentNode);
                return -ENOTDIR;
            }

            // Check if the next node is already cached
            struct ts_list *l_node = l_currentNode->m_specific.m_children;

            while(l_node != NULL) {
                struct ts_vfsNode *l_listElementNode = l_node->m_data;

                if(kstrcmp(l_listElementNode->m_name, l_name) == 0) {
                    break;
                } else {
                    l_node = l_node->m_next;
                }
            }

            // If the node was not found in cache, get it using the filesystem's
            // lookup() operation.
            if(l_node == NULL) {
                if(
                    l_currentNode->m_operations == NULL
                    || l_currentNode->m_operations->m_lookup == NULL
                ) {
                    return -ENOTSUP;
                }

                struct ts_vfsNode *l_nextNode;

                int l_returnValue = l_currentNode->m_operations->m_lookup(
                    l_currentNode,
                    l_name,
                    &l_nextNode
                );

                if(l_returnValue != 0) {
                    vfsRelease(l_currentNode);
                    return l_returnValue;
                } else {
                    listAdd(&l_currentNode->m_specific.m_children, l_nextNode);
                    l_currentNode = l_nextNode;
                    l_currentNode->m_referenceCount = 1;
                }
            } else {
                l_currentNode = l_node->m_data;

                // Increment the reference count
                if(l_currentNode->m_referenceCount >= 0) {
                    l_currentNode->m_referenceCount++;
                }
            }
            
            if(p_path[l_index] == '\0') {
                *p_result = l_currentNode;
                return 0;
            }
        }

        l_index++;
    }
}

int vfsLookupParent(const char *p_path, struct ts_vfsNode **p_result) {
    if(p_path[0] != '/') {
        return -EINVAL;
    }
    
    if(p_path[1] == '\0') {
        return vfsLookup("/", p_result);
    }

    char *l_lastPathSeparator = kstrrchr(p_path, '/');
    
    if(l_lastPathSeparator == p_path) {
        return vfsLookup("/", p_result);
    }

    size_t l_parentPathLength = l_lastPathSeparator - p_path;

    if(l_parentPathLength > PATH_MAX) {
        return -EINVAL;
    }

    char l_parentPath[l_parentPathLength + 1];

    kmemcpy(l_parentPath, p_path, l_parentPathLength);

    l_parentPath[l_parentPathLength] = '\0';

    return vfsLookup(l_parentPath, p_result);
}

int vfsRelease(struct ts_vfsNode *p_node) {
    // If the node cannot be released, don't decrement the reference count.
    if(p_node->m_referenceCount == -1) {
        return -1;
    }

    p_node->m_referenceCount--;

    if(p_node->m_referenceCount == 0) {
        // Call the file system's release() operation if the node reference
        // count reaches 0.
        if(
            p_node->m_operations != NULL
            && p_node->m_operations->m_release != NULL
        ) {
            int l_returnValue = p_node->m_operations->m_release(p_node);

            if(l_returnValue != 0) {
                return l_returnValue;
            }
        }

        struct ts_vfsNode *l_parent = p_node->m_parent;

        listRemove(&l_parent->m_specific.m_children, p_node);

        kfree(p_node);

        // We also call release() on the parent because a node always counts
        // for one reference in the parent.
        if(l_parent != NULL) {
            vfsRelease(l_parent);
        }
    }

    return 0;
}

int vfsOpen(
    struct ts_vfsNode *p_node,
    int p_flags,
    struct ts_vfsFile **p_file
) {
    struct ts_vfsFile *l_file;

    if(p_node->m_operations == NULL || p_node->m_operations->m_open == NULL) {
        return -ENOTSUP;
    }

    // TODO: checks

    l_file = kmalloc(sizeof(struct ts_vfsFile));

    if(l_file == NULL) {
        return -ENOMEM;
    }

    l_file->m_vfsNode = p_node;
    l_file->m_flags = p_flags;

    int l_returnValue = p_node->m_operations->m_open(
        p_node,
        p_flags,
        l_file
    );

    if(l_returnValue != 0) {
        kfree(l_file);
        return l_returnValue;
    }

    if(p_node->m_referenceCount != -1) {
        p_node->m_referenceCount++;
    }

    *p_file = l_file;

    return 0;
}

int vfsChmod(const char *p_path, mode_t p_mode) {
    if(!vfsIsPathValid(p_path)) {
        return -EINVAL;
    }

    struct ts_vfsNode *l_node;
    int l_returnValue = vfsLookup(p_path, &l_node);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    if(l_node->m_operations == NULL || l_node->m_operations->m_chmod == NULL) {
        vfsRelease(l_node);
        return -ENOTSUP;
    }

    l_returnValue = l_node->m_operations->m_chmod(l_node, p_mode);

    vfsRelease(l_node);

    if(l_returnValue == 0) {
        l_node->m_mod = p_mode;
    }

    return l_returnValue;
}

int vfsChown(const char *p_path, uid_t p_owner) {
    if(!vfsIsPathValid(p_path)) {
        return -EINVAL;
    }

    struct ts_vfsNode *l_node;
    int l_returnValue = vfsLookup(p_path, &l_node);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    if(l_node->m_operations == NULL || l_node->m_operations->m_chown == NULL) {
        vfsRelease(l_node);
        return -ENOTSUP;
    }

    l_returnValue = l_node->m_operations->m_chown(l_node, p_owner);

    vfsRelease(l_node);

    if(l_returnValue == 0) {
        l_node->m_uid = p_owner;
    }

    return l_returnValue;
}

int vfsChgrp(const char *p_path, gid_t p_group) {
    if(!vfsIsPathValid(p_path)) {
        return -EINVAL;
    }

    struct ts_vfsNode *l_node;
    int l_returnValue = vfsLookup(p_path, &l_node);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    if(l_node->m_operations == NULL || l_node->m_operations->m_chgrp == NULL) {
        vfsRelease(l_node);
        return -ENOTSUP;
    }

    l_returnValue = l_node->m_operations->m_chgrp(l_node, p_group);

    vfsRelease(l_node);

    if(l_returnValue == 0) {
        l_node->m_gid = p_group;
    }

    return l_returnValue;
}

int vfsLink(const char *p_path1, const char *p_path2) {
    if(!vfsIsPathValid(p_path1) || !vfsIsPathValid(p_path2)) {
        return -EINVAL;
    }

    const char *l_name = kstrrchr(p_path2, '/') + 1;
    
    struct ts_vfsNode *l_node;
    int l_returnValue = vfsLookup(p_path1, &l_node);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    struct ts_vfsNode *l_parentNode;
    l_returnValue = vfsLookupParent(p_path2, &l_parentNode);

    if(l_returnValue != 0) {
        vfsRelease(l_node);
        return l_returnValue;
    }

    if(l_node->m_operations == NULL || l_node->m_operations->m_link == NULL) {
        vfsRelease(l_node);
        vfsRelease(l_parentNode);
        return -ENOTSUP;
    }

    l_returnValue = l_node->m_operations->m_link(l_node, l_parentNode, l_name);

    vfsRelease(l_node);
    vfsRelease(l_parentNode);

    return l_returnValue;
}

int vfsUnlink(const char *p_path) {
    if(!vfsIsPathValid(p_path)) {
        return -EINVAL;
    }

    struct ts_vfsNode *l_node;
    int l_returnValue = vfsLookup(p_path, &l_node);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    if(l_node->m_operations == NULL || l_node->m_operations->m_unlink == NULL) {
        vfsRelease(l_node);
        return -ENOTSUP;
    }

    l_returnValue = l_node->m_operations->m_unlink(l_node);

    vfsRelease(l_node);

    return l_returnValue;
}

int vfsMknod(const char *p_path, mode_t p_mode, dev_t p_device) {
    if(!vfsIsPathValid(p_path)) {
        return -EINVAL;
    }

    const char *l_name = kstrrchr(p_path, '/') + 1;
    
    struct ts_vfsNode *l_node;
    int l_returnValue = vfsLookupParent(p_path, &l_node);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    if(l_node->m_operations == NULL || l_node->m_operations->m_mknod == NULL) {
        vfsRelease(l_node);
        return -ENOTSUP;
    }

    l_returnValue = l_node->m_operations->m_mknod(
        l_node,
        l_name,
        p_mode,
        p_device
    );

    vfsRelease(l_node);

    return l_returnValue;
}

int vfsFileClose(struct ts_vfsFile *p_file) {
    if(
        p_file->m_operations != NULL
        && p_file->m_operations->m_close != NULL
    ) {
        int l_result = p_file->m_operations->m_close(p_file);

        if(l_result != 0) {
            return l_result;
        }
    }

    vfsRelease(p_file->m_vfsNode);

    kfree(p_file);

    return 0;
}

int vfsFileLlseek(struct ts_vfsFile *p_file, loff_t p_offset, int p_whence) {
    if(p_whence < 0 || p_whence > 2) {
        return -EINVAL;
    }

    if(p_file->m_operations == NULL || p_file->m_operations->m_llseek == NULL) {
        return -ENOTSUP;
    }

    return p_file->m_operations->m_llseek(p_file, p_offset, p_whence);
}

int vfsFileIoctl(struct ts_vfsFile *p_file, int p_request, void *p_arg) {
    if(p_file->m_operations == NULL || p_file->m_operations->m_ioctl == NULL) {
        return -ENOTSUP;
    }

    return p_file->m_operations->m_ioctl(p_file, p_request, p_arg);
}

ssize_t vfsFileRead(struct ts_vfsFile *p_file, void *p_buffer, size_t p_size) {
    if(p_buffer == NULL) {
        return -EINVAL;
    }

    if(p_file->m_operations == NULL || p_file->m_operations->m_read == NULL) {
        return -ENOTSUP;
    }

    return p_file->m_operations->m_read(p_file, p_buffer, p_size);
}

ssize_t vfsFileWrite(
    struct ts_vfsFile *p_file,
    const void *p_buffer,
    size_t p_size
) {
    if(p_buffer == NULL) {
        return -EINVAL;
    }

    if(p_file->m_operations == NULL || p_file->m_operations->m_write == NULL) {
        return -ENOTSUP;
    }

    return p_file->m_operations->m_write(p_file, p_buffer, p_size);
}

int vfsRegisterFileSystem(const struct ts_vfsFileSystem *p_fileSystem) {
    if(vfsGetFileSystem(p_fileSystem->m_name) != NULL) {
        return -EBUSY;
    }

    return listAdd(&s_vfsFileSystemList, (void *)p_fileSystem);
}

int vfsUnregisterFileSystem(const struct ts_vfsFileSystem *p_fileSystem) {
    if(vfsGetFileSystem(p_fileSystem->m_name) == NULL) {
        return -EINVAL;
    }

    return listRemove(&s_vfsFileSystemList, (void *)p_fileSystem);
}

const struct ts_vfsFileSystem *vfsGetFileSystem(const char *p_name) {
    struct ts_list *l_listElement = s_vfsFileSystemList;

    while(l_listElement != NULL) {
        const struct ts_vfsFileSystem *l_fileSystem = l_listElement->m_data;

        if(kstrcmp(l_fileSystem->m_name, p_name) == 0) {
            return l_fileSystem;
        }

        l_listElement = l_listElement->m_next;
    }

    return NULL;
}

static int vfsCanonicalizePath(const char *p_path, char *p_buffer) {
    size_t l_pathLength = kstrlen(p_path);

    // Check the length of the path
    if(l_pathLength > PATH_MAX) {
        return -EINVAL;
    }

    // The path must start with a "/".
    if(p_path[0] != '/') {
        return -EINVAL;
    }

    struct {
        int m_start;
        int m_length;
    } l_pathStack[PATH_MAX / 2];
    int l_pathStackIndex = 0;
    int l_index = 1;
    int l_nameStartIndex = 1;

    // Place all path elements into a stack.
    while(true) {
        if(p_path[l_index] == '/') {
            const int l_nameLength = l_index - l_nameStartIndex;

            l_index++;

            if(l_nameLength == 0) {
                // "a//" => "a/"
            } else {
                if(l_nameLength == 1 && p_path[l_nameStartIndex] == '.') {
                    // "a/./" => "a/"
                } else if(
                    l_nameLength == 2
                    && p_path[l_nameStartIndex] == '.'
                    && p_path[l_nameStartIndex + 1] == '.'
                ) {
                    // "a/../" => "/"
                    if(l_pathStackIndex == 0) {
                        return -EINVAL;
                    } else {
                        l_pathStackIndex--;
                    }
                } else {
                    l_pathStack[l_pathStackIndex].m_start = l_nameStartIndex;
                    l_pathStack[l_pathStackIndex].m_length = l_nameLength;
                    l_pathStackIndex++;
                }
            }

            l_nameStartIndex = l_index;
        } else if(p_path[l_index] == '\0') {
            l_pathStack[l_pathStackIndex].m_start = l_nameStartIndex;
            l_pathStack[l_pathStackIndex].m_length = l_index - l_nameStartIndex;
            l_pathStackIndex++;
            break;
        } else {
            l_index++;
        }
    }

    int l_bufferIndex = 0;

    for(int l_i = 0; l_i < l_pathStackIndex; l_i++) {
        int l_length = l_pathStack[l_i].m_length;

        // This condition prevents the "/" from being added at the end.
        if(l_length != 0) {
            p_buffer[l_bufferIndex++] = '/';

            kmemcpy(
                &p_buffer[l_bufferIndex],
                &p_path[l_pathStack[l_i].m_start],
                l_length
            );

            l_bufferIndex += l_length;
        }
    }

    p_buffer[l_bufferIndex] = '\0';

    return 0;
}

static bool vfsIsPathValid(const char *p_path) {
    size_t l_pathLength = kstrlen(p_path);

    if(l_pathLength > PATH_MAX) {
        return false;
    }

    if(p_path[0] != '/') {
        return false;
    }

    if(p_path[l_pathLength - 1] == '/') {
        return false;
    }

    return true;
}
