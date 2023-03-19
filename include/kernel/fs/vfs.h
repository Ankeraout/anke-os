#ifndef __INCLUDE_KERNEL_FS_VFS_H__
#define __INCLUDE_KERNEL_FS_VFS_H__

#include <kernel/misc/spinlock.h>
#include <kernel/misc/tree.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/types.h>

enum te_vfsNodeType {
    E_VFSNODETYPE_FILE,
    E_VFSNODETYPE_DIRECTORY,
    E_VFSNODETYPE_CHARACTERDEVICE
};

struct ts_vfsNode;

typedef int tf_vfsClose(struct ts_vfsNode *p_node);
typedef int tf_vfsCreate(
    struct ts_vfsNode *p_node,
    const char *p_name
);
typedef int tf_vfsIoctl(
    struct ts_vfsNode *p_node,
    int p_request,
    void *p_arg
);
typedef int tf_vfsLookup(
    struct ts_vfsNode *p_node,
    const char *p_name,
    struct ts_vfsNode **p_output
);
typedef int tf_vfsMkdir(
    struct ts_vfsNode *p_node,
    const char *p_name
);
typedef int tf_vfsMknod(
    struct ts_vfsNode *p_node,
    const char *p_name,
    enum te_vfsNodeType p_type,
    dev_t p_deviceNumber
);
typedef int tf_vfsOpen(struct ts_vfsNode *p_node, int p_flags);
typedef ssize_t tf_vfsRead(
    struct ts_vfsNode *p_node,
    off_t p_offset,
    void *p_buffer,
    size_t p_size
);
typedef int tf_vfsRemove(struct ts_vfsNode *p_node, const char *p_name);
typedef int tf_vfsRmdir(struct ts_vfsNode *p_node, const char *p_name);
typedef ssize_t tf_vfsWrite(
    struct ts_vfsNode *p_node,
    off_t p_offset,
    const void *p_buffer,
    size_t p_size
);

struct ts_vfsNodeOperations {
    tf_vfsClose *a_close;
    tf_vfsCreate *a_create;
    tf_vfsIoctl *a_ioctl;
    tf_vfsLookup *a_lookup;
    tf_vfsMkdir *a_mkdir;
    tf_vfsMknod *a_mknod;
    tf_vfsOpen *a_open;
    tf_vfsRead *a_read;
    tf_vfsRemove *a_remove;
    tf_vfsRmdir *a_rmdir;
    tf_vfsWrite *a_write;
};

struct ts_vfsTreeNode;

struct ts_vfsNode {
    t_spinlock a_lock;
    int a_referenceCount;
    enum te_vfsNodeType a_type;
    struct ts_treeNode *a_vfs;
    const struct ts_vfsNodeOperations *a_operations;
    dev_t a_deviceNumber;
    void *a_fsData;
};

struct ts_vfsFileSystem {
    const char *a_name;
    const struct ts_vfsNodeOperations *a_operations;
    int (*a_onMount)(struct ts_vfsNode *p_node, dev_t p_device);
};

/**
 * @brief Creates a new VFS node.
 *
 * @param[in] p_parent The parent of the new VFS node. Setting this parameter to
 * NULL means that the new VFS node is a child of the root node.
 * @param[in] p_name The name of the new VFS node.
 * @param[in] p_autoFree A boolean value that indicates whether this node will
 * use the reference counter. The reference counter should be used for every
 * node but the root node and mount point nodes.
 * @param[out] p_output The created node.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success
 * @retval Any other value on error.
 */
int vfsCreateNode(
    struct ts_vfsNode *p_parent,
    const char *p_name,
    bool p_autoFree,
    struct ts_vfsNode **p_output
);

void vfsDebug(void);

/**
 * @brief Initializes the VFS.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success
 * @retval Any other value on error.
*/
int vfsInit(void);

/**
 * @brief Looks for a file and returns its node.
 *
 * @param[in] p_cwd The current working directory. This will be ignored (can be
 * set to NULL) if p_path is an absolute path.
 * @param[in] p_path The path of the file to look for.
 * @param[out] p_output The node of the searched file.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success
 * @retval Any other value on error.
 */
int vfsLookup(
    const char *p_cwd,
    const char *p_path,
    struct ts_vfsNode **p_output
);

/**
 * @brief Mounts a file system.
 *
 * @param[in] p_node The node to mount the file system in.
 * @param[in] p_fileSystem The file system to mount.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success
 * @retval Any other value on error.
 */
int vfsMount(struct ts_vfsNode *p_node, const char *p_fileSystem);

/**
 * @brief Closes the given node.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success
 * @retval Any other value on error.
 */
int vfsOperationClose(struct ts_vfsNode *p_node);

/**
 * @brief Calls ioctl() on the given file.
 *
 * @param[in] p_node The file node.
 * @param[in] p_request The request number.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success
 * @retval Any other value on error.
 */
int vfsOperationIoctl(
    struct ts_vfsNode *p_node,
    int p_request,
    void *p_arg
);

/**
 * @brief Looks for a child file and returns its node.
 *
 * @param[in] p_node The node to search in.
 * @param[in] p_name The name of the file to look for.
 * @param[out] p_output The node of the searched file.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success
 * @retval Any other value on error.
 */
int vfsOperationLookup(
    struct ts_vfsNode *p_node,
    const char *p_name,
    struct ts_vfsNode **p_output
);

/**
 * @brief Creates a directory.
 *
 * @param[in] p_node The node to create the directory in. This node must be a
 * directory.
 * @param[in] p_name The name of the new directory.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success
 * @retval Any other value on error.
*/
int vfsOperationMkdir(
    struct ts_vfsNode *p_node,
    const char *p_name
);

/**
 * @brief Creates a new special file.
 *
 * @param[in] p_node The node to create the file in. This node must be a
 * directory.
 * @param[in] p_name The name of the new file.
 * @param[in] p_type The type of the new file. The only accepted value currently
 * is E_VFSNODETYPE_CHARACTERDEVICE.
 * @param[in] p_deviceNumber The device number (for block and character
 * devices).
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success
 * @retval Any other value on error.
*/
int vfsOperationMknod(
    struct ts_vfsNode *p_node,
    const char *p_name,
    enum te_vfsNodeType p_type,
    dev_t p_deviceNumber
);

/**
 * @brief Reads data from the given file.
 *
 * @param[in] p_node The file to read the data from.
 * @param[in] p_offset The offset in the data stream.
 * @param[in] p_buffer The buffer that will receive the data.
 * @param[in] p_size The maximum number of bytes to read.
 *
 * @returns An integer that indicates the result of the operation. If it is
 * negative, then it is an error code. If it is positive or null, then it
 * contains the number of bytes read.
 */
ssize_t vfsOperationRead(
    struct ts_vfsNode *p_node,
    off_t p_offset,
    void *p_buffer,
    size_t p_size
);

/**
 * @brief Writes the given data to the given file.
 *
 * @param[in] p_node The file to write the data to.
 * @param[in] p_offset The offset in the data stream.
 * @param[in] p_buffer The data to write.
 * @param[in] p_size The number of bytes to write.
 *
 * @returns An integer that indicates the result of the operation. If it is
 * negative, then it is an error code. If it is positive or null, then it
 * contains the number of bytes written.
 */
ssize_t vfsOperationWrite(
    struct ts_vfsNode *p_node,
    off_t p_offset,
    const void *p_buffer,
    size_t p_size
);

/**
 * @brief Registers a file system in the file system list.
 *
 * @param[in] p_fileSystem The file system to register.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success
 * @retval Any other value on error.
 */
int vfsRegisterFileSystem(const struct ts_vfsFileSystem *p_fileSystem);

#endif
