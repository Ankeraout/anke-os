#include "errno.h"
#include "list.h"
#include "stdlib.h"
#include "string.h"
#include "tree.h"
#include "vfs/vfs.h"

static struct ts_vfsVnode *s_rootNode;
static struct ts_listNode *s_fileSystemList;

int vfs_init(void) {
    s_rootNode = NULL;
    s_fileSystemList = NULL;
    return 0;
}

int vfs_mount(
    struct ts_vfsInode *p_inode,
    struct ts_vfsVnode *p_target,
    struct ts_vfsFileSystem *p_fileSystem,
    int p_flags
) {
    struct ts_vfsVnode *l_target;

    if(p_target == NULL) {
        // We're trying to mount root
        
        if(s_rootNode != NULL) {
            // Root is already mounted
            return -EBUSY;
        }

        l_target = malloc(sizeof(struct ts_vfsVnode));

        if(l_target == NULL) {
            return -ENOMEM;
        }

        struct ts_treeNode *l_treeNode = malloc(sizeof(struct ts_treeNode));

        if(l_treeNode == NULL) {
            free(l_target);
            return -ENOMEM;
        }

        memset(l_target, 0, sizeof(struct ts_vfsVnode));
        memset(l_treeNode, 0, sizeof(struct ts_treeNode));
        
        l_target->m_treeNode = l_treeNode;
        strcpy(l_target->m_name, "[ROOT]");
        l_target->m_referenceCount = 1;

        l_treeNode->m_data = l_target;
    } else {
        l_target = p_target;
    }

    int l_result = 
        p_fileSystem->m_mount(p_fileSystem, p_inode, l_target, p_flags);

    if(p_target == NULL) {
        if(l_result < 0) {
            free(l_target->m_treeNode);
            free(l_target);
        } else {
            s_rootNode = l_target;
        }
    }

    return l_result;
}

int vfs_registerFileSystem(struct ts_vfsFileSystem *p_fileSystem) {
    return list_insertBeginning(&s_fileSystemList, p_fileSystem);
}

struct ts_vfsFileSystem *vfs_getFileSystemByName(const char *p_name) {
    struct ts_listNode *l_node = s_fileSystemList;

    while(l_node != NULL) {
        struct ts_vfsFileSystem *l_fileSystem =
            (struct ts_vfsFileSystem *)l_node->m_data;

        if(strcmp(l_fileSystem->m_name, p_name) == 0) {
            return l_fileSystem;
        }

        l_node = l_node->m_next;
    }

    return NULL;
}
