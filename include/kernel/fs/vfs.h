#ifndef __INCLUDE_KERNEL_FS_VFS_H__
#define __INCLUDE_KERNEL_FS_VFS_H__

#define C_VFS_MAX_FILENAME_SIZE 256

struct ts_vfsNode;

typedef int tf_vfsNodeOpen(struct ts_vfsNode *p_node);
typedef int tf_vfsNodeClose(struct ts_vfsNode *p_node);

enum te_vfsNodeType {
    E_VFSNODETYPE_FILE,
    E_VFSNODETYPE_FOLDER,
    E_VFSNODETYPE_MOUNTPOINT
};

struct ts_vfsNode {
    char a_name[C_VFS_MAX_FILENAME_SIZE];
    enum te_vfsNodeType a_type;
    tf_vfsNodeOpen *a_open;
};

int vfsInit(void);
struct ts_vfsNode *vfsOpen(const char *p_path);
void vfsClose(struct ts_vfsNode *p_node);

#endif
