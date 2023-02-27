#include <kernel/debug.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/misc/tree.h>

static struct ts_treeNode s_vfsRootNode;

int vfsInit(void) {
    struct ts_vfsNode *l_rootNode =
        kmalloc(sizeof(struct ts_vfsNode));

    if(l_rootNode == NULL) {
        debug("vfs: Failed to allocate memory for root node information.\n");
        return 1;
    }

    l_rootNode->a_type = E_VFSNODETYPE_FOLDER;

    if(treeInit(&s_vfsRootNode, l_rootNode) != 0) {
        debug("vfs: Failed to allocate memory for root node.\n");
        kfree(l_rootNode);
        return 1;
    }

    debug("vfs: VFS initialized.\n");

    return 0;
}
