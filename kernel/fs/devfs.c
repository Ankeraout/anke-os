#include <errno.h>
#include <string.h>

#include <kernel/common.h>
#include <kernel/debug.h>
#include <kernel/fs/devfs.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/misc/list.h>

static struct ts_vfsFileDescriptor *devfsOpen(
    struct ts_vfsFileDescriptor *p_file,
    const char *p_path,
    int p_flags
);
static int devfsIoctl(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
);

struct ts_vfsFileDescriptor *devfsInit(void) {
    struct ts_vfsFileDescriptor *l_fileDescriptor =
        kcalloc(sizeof(struct ts_vfsFileDescriptor));

    if(l_fileDescriptor == NULL) {
        return NULL;
    }

    struct ts_linkedList *l_fileList = kmalloc(sizeof(struct ts_linkedList));

    if(l_fileList == NULL) {
        kfree(l_fileDescriptor);
        return NULL;
    }

    if(linkedListInit(l_fileList) != 0) {
        kfree(l_fileList);
        kfree(l_fileDescriptor);
        return NULL;
    }

    strcpy(l_fileDescriptor->a_name, "dev");
    l_fileDescriptor->a_ioctl = devfsIoctl;
    l_fileDescriptor->a_open = devfsOpen;
    l_fileDescriptor->a_context = l_fileList;

    return l_fileDescriptor;
}

static struct ts_vfsFileDescriptor *devfsOpen(
    struct ts_vfsFileDescriptor *p_file,
    const char *p_path,
    int p_flags
) {
    M_UNUSED_PARAMETER(p_flags);

    if(p_path[0] == '\0') {
        return p_file;
    }

    // Look for a file with the given name
    struct ts_linkedList *l_fileList = p_file->a_context;
    struct ts_linkedListNode *l_node = l_fileList->a_first;

    while(l_node != NULL) {
        struct ts_vfsFileDescriptor *l_nodeFile = l_node->a_data;

        if(strcmp(l_nodeFile->a_name, p_path) == 0) {
            break;
        }

        l_node = l_node->a_next;
    }

    if(l_node == NULL) {
        return NULL;
    }

    return l_node->a_data;
}

static int devfsIoctl(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
) {
    struct ts_linkedList *l_fileList = p_file->a_context;

    if(p_request == C_IOCTL_DEVFS_CREATE) {
        if(linkedListAdd(l_fileList, p_arg) != 0) {
            return -ENOMEM;
        }
    } else {
        return -EINVAL;
    }

    return 0;
}
