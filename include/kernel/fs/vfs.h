#ifndef __INCLUDE_KERNEL_FS_VFS_H__
#define __INCLUDE_KERNEL_FS_VFS_H__

#include <stdio.h>

#include <kernel/misc/spinlock.h>
#include <sys/types.h>

#define C_VFS_MAX_FILENAME_SIZE 256

enum {
    E_VFS_FILETYPE_FILE = 1,
    E_VFS_FILETYPE_FOLDER,
    E_VFS_FILETYPE_BLOCK,
    E_VFS_FILETYPE_CHARACTER
};

struct ts_vfsFileDescriptor;

typedef struct ts_vfsFileDescriptor *tf_vfsFind(
    struct ts_vfsFileDescriptor *p_file,
    const char *p_path
);
typedef int tf_vfsOpen(struct ts_vfsFileDescriptor *p_file, int p_flags);
typedef void tf_vfsClose(struct ts_vfsFileDescriptor *p_file);
typedef ssize_t tf_vfsRead(
    struct ts_vfsFileDescriptor *p_file,
    void *p_buffer,
    size_t p_size
);
typedef ssize_t tf_vfsWrite(
    struct ts_vfsFileDescriptor *p_file,
    const void *p_buffer,
    size_t p_size
);
typedef int tf_vfsIoctl(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
);
typedef off_t tf_vfsLseek(
    struct ts_vfsFileDescriptor *p_file,
    off_t p_offset,
    int p_whence
);

/**
 * @brief This structure represents a file descriptor.
 */
struct ts_vfsFileDescriptor {
    // Free for use by the FS driver.
    void *a_context;

    int a_openFlags;
    char a_name[C_VFS_MAX_FILENAME_SIZE];
    int a_type;
    tf_vfsOpen *a_open;
    tf_vfsFind *a_find;
    tf_vfsClose *a_close;
    tf_vfsRead *a_read;
    tf_vfsWrite *a_write;
    tf_vfsIoctl *a_ioctl;
    tf_vfsLseek *a_lseek;
};

/**
 * @brief This structure represents a VFS tree node. Note that a VFS tree node
 *        must always have a name. The a_fileDescriptor field is NULL unless the
 *        node is a mount point or a special file.
 */
struct ts_vfsNode {
    char *a_name;
    struct ts_vfsFileDescriptor *a_fileDescriptor;
};

/**
 * @brief Prints the VFS tree.
 */
void vfsDebug(void);

/**
 * @brief Initializes the VFS.
 */
int vfsInit(void);

/**
 * @brief Gets the mount point for the given path.
 *
 * @param[in] p_path The path to find the mount point of
 * @param[out] p_relativePath The path relative to the mount point
 *
 * @returns The mount point file descriptor
 * @retval NULL if the mount point could not be found or if the path was
 *         invalid.
 */
struct ts_vfsFileDescriptor *vfsGetMountPoint(
    const char *p_path,
    const char **p_relativePath
);

/**
 * @brief Mounts the given file descriptor at the given path.
 *
 * @retval 0 if the file was mounted successfully
 * @retval Any other value if an error occurred
 */
int vfsMount(
    const char *p_mountPath,
    struct ts_vfsFileDescriptor *p_fileDescriptor
);

/**
 * @brief Gets the file descriptor for the given path.
 *
 * @retval NULL if the file could not be opened.
 */
struct ts_vfsFileDescriptor *vfsFind(const char *p_path);

/**
 * @brief Clones the given file descriptor.
 *
 * @retval NULL if an error occurred.
 */
struct ts_vfsFileDescriptor *vfsClone(struct ts_vfsFileDescriptor *p_file);

#endif
