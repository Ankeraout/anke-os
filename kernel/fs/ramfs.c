#include <errno.h>
#include <kernel/common.h>
#include <kernel/dev/device.h>
#include <kernel/fs/ramfs.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/misc/spinlock.h>
#include <kernel/misc/tree.h>
#include <limits.h>
#include <string.h>

struct ts_ramfsFile {
    enum te_vfsNodeType a_type;
    char a_name[NAME_MAX + 1];
    dev_t a_deviceNumber;
    t_spinlock a_lock;
};

static void ramfsInitFile(struct ts_ramfsFile *p_file);
static struct ts_treeNode *ramfsLookup(
    struct ts_treeNode *p_node,
    const char *p_name
);
static int ramfsOperationLookup(
    struct ts_vfsNode *p_node,
    const char *p_name,
    struct ts_vfsNode **p_output
);
static int ramfsOnMount(struct ts_vfsNode *p_node, dev_t p_device);
static int ramfsOperationMkdir(
    struct ts_vfsNode *p_node,
    const char *p_name
);
static int ramfsOperationMknod(
    struct ts_vfsNode *p_node,
    const char *p_name,
    enum te_vfsNodeType p_type,
    dev_t p_deviceNumber
);

static const struct ts_vfsNodeOperations s_ramfsOperations = {
    .a_lookup = ramfsOperationLookup,
    .a_mkdir = ramfsOperationMkdir,
    .a_mknod = ramfsOperationMknod
};

const struct ts_vfsFileSystem g_ramfsFileSystem = {
    .a_name = "ramfs",
    .a_onMount = ramfsOnMount,
    .a_operations = &s_ramfsOperations
};

static void ramfsInitFile(struct ts_ramfsFile *p_file) {
    spinlockInit(&p_file->a_lock);
}

static struct ts_treeNode *ramfsLookup(
    struct ts_treeNode *p_node,
    const char *p_name
) {
    struct ts_linkedListNode *l_child = p_node->a_children.a_first;

    while(l_child != NULL) {
        struct ts_treeNode *l_childTreeNode = l_child->a_data;
        struct ts_ramfsFile *l_childFile = l_childTreeNode->a_data;

        if(strcmp(l_childFile->a_name, p_name) == 0) {
            break;
        }

        l_child = l_child->a_next;
    }

    if(l_child == NULL) {
        return NULL;
    }

    return l_child->a_data;
}

static int ramfsOperationLookup(
    struct ts_vfsNode *p_node,
    const char *p_name,
    struct ts_vfsNode **p_output
) {
    struct ts_treeNode *l_node = p_node->a_fsData;
    struct ts_ramfsFile *l_file = l_node->a_data;

    spinlockAcquire(&l_file->a_lock);

    struct ts_treeNode *l_childTreeNode = ramfsLookup(l_node, p_name);

    spinlockRelease(&l_file->a_lock);

    if(l_childTreeNode == NULL) {
        return -ENOENT;
    }

    struct ts_vfsNode *l_childNode;
    int l_returnValue = vfsCreateNode(p_node, p_name, true, &l_childNode);

    if(l_returnValue != 0) {
        spinlockRelease(&l_file->a_lock);
        return l_returnValue;
    }

    struct ts_ramfsFile *l_childFile = l_childTreeNode->a_data;

    l_childNode->a_type = l_childFile->a_type;
    l_childNode->a_fsData = l_childTreeNode;
    l_childNode->a_deviceNumber = l_childFile->a_deviceNumber;

    if(
        (l_childNode->a_type == E_VFSNODETYPE_FILE)
        || (l_childNode->a_type == E_VFSNODETYPE_DIRECTORY)
    ) {
        l_childNode->a_operations = &s_ramfsOperations;
    }

    switch(l_childNode->a_type) {
        case E_VFSNODETYPE_FILE:
        case E_VFSNODETYPE_DIRECTORY:
            l_childNode->a_operations = &s_ramfsOperations;
            break;

        case E_VFSNODETYPE_CHARACTERDEVICE:
            l_returnValue = deviceGetOperations(
                l_childFile->a_deviceNumber,
                &l_childNode->a_operations
            );

            if(l_returnValue != 0) {
                return l_returnValue;
            }

            break;
    }

    *p_output = l_childNode;

    return 0;
}

static int ramfsOnMount(struct ts_vfsNode *p_node, dev_t p_device) {
    // The p_device parameter is not used. Ramfs is always empty when mounted.
    M_UNUSED_PARAMETER(p_device);

    // Allocate memory for the file system tree.
    p_node->a_fsData = kmalloc(sizeof(struct ts_treeNode));

    if(p_node->a_fsData == NULL) {
        return -ENOMEM;
    }

    // Initialize the ramfs root directory information structure
    struct ts_ramfsFile *l_file = kmalloc(sizeof(struct ts_ramfsFile));

    if(l_file == NULL) {
        kfree(p_node->a_fsData);
        return -ENOMEM;
    }

    // Initialize the root file system tree node.
    struct ts_treeNode *l_node = p_node->a_fsData;

    if(treeInit(l_node, l_file) != 0) {
        kfree(p_node->a_fsData);
        kfree(l_file);
        return -ENOMEM;
    }

    return 0;
}

static int ramfsOperationMkdir(
    struct ts_vfsNode *p_node,
    const char *p_name
) {
    struct ts_treeNode *l_node = p_node->a_fsData;
    struct ts_ramfsFile *l_parentFile = l_node->a_data;

    spinlockAcquire(&l_parentFile->a_lock);

    // Check if the file does not exist yet
    if(ramfsLookup(l_node, p_name) != NULL) {
        spinlockRelease(&l_parentFile->a_lock);
        return -EEXIST;
    }

    // Create a file structure.
    struct ts_ramfsFile *l_file = kmalloc(sizeof(struct ts_ramfsFile));

    if(l_file == NULL) {
        spinlockRelease(&l_parentFile->a_lock);
        return -ENOMEM;
    }

    ramfsInitFile(l_file);
    l_file->a_type = E_VFSNODETYPE_DIRECTORY;
    strcpy(l_file->a_name, p_name);

    // Add the file to the parent node.
    struct ts_treeNode *l_parentNode = p_node->a_fsData;

    if(treeAddChild(l_parentNode, l_file) == NULL) {
        spinlockRelease(&l_parentFile->a_lock);
        kfree(l_file);
        return -ENOMEM;
    }

    spinlockRelease(&l_parentFile->a_lock);

    return 0;
}

static int ramfsOperationMknod(
    struct ts_vfsNode *p_node,
    const char *p_name,
    enum te_vfsNodeType p_type,
    dev_t p_deviceNumber
) {
    // Ramfs only supports character devices.
    if(p_type != E_VFSNODETYPE_CHARACTERDEVICE) {
        return -EINVAL;
    }

    struct ts_treeNode *l_node = p_node->a_fsData;
    struct ts_ramfsFile *l_parentFile = l_node->a_data;

    spinlockAcquire(&l_parentFile->a_lock);

    // Check if the file does not exist yet
    if(ramfsLookup(l_node, p_name) != NULL) {
        spinlockRelease(&l_parentFile->a_lock);
        return -EEXIST;
    }

    // Create a file structure.
    struct ts_ramfsFile *l_file = kmalloc(sizeof(struct ts_ramfsFile));

    if(l_file == NULL) {
        spinlockRelease(&l_parentFile->a_lock);
        return -ENOMEM;
    }

    ramfsInitFile(l_file);
    l_file->a_deviceNumber = p_deviceNumber;
    l_file->a_type = p_type;
    strcpy(l_file->a_name, p_name);

    // Add the file to the parent node.
    struct ts_treeNode *l_parentNode = p_node->a_fsData;

    if(treeAddChild(l_parentNode, l_file) == NULL) {
        spinlockRelease(&l_parentFile->a_lock);
        kfree(l_file);
        return -ENOMEM;
    }

    spinlockRelease(&l_parentFile->a_lock);

    return 0;
}
