#ifndef __INCLUDE_KERNEL_FS_VFS_H__
#define __INCLUDE_KERNEL_FS_VFS_H__

#include <stddef.h>
#include <stdint.h>

#include "sys/types.h"
#include "util/list.h"

#define C_MAX_FILE_NAME_SIZE 255

enum te_vfsNodeType {
    E_VFSNODETYPE_FILE,
    E_VFSNODETYPE_DIRECTORY,
    E_VFSNODETYPE_CHAR,
    E_VFSNODETYPE_BLOCK,
    E_VFSNODETYPE_MOUNT
};

struct ts_vfsFileOperations;

struct ts_vfsFile {
    struct ts_vfsNode *m_vfsNode;
    struct ts_vfsFileOperations *m_operations;
    int m_flags;
};

struct ts_vfsFileOperations {
    ssize_t (*m_read)(
        struct ts_vfsFile *p_file,
        void *p_buffer,
        size_t p_size
    );
    ssize_t (*m_write)(
        struct ts_vfsFile *p_file,
        const void *p_buffer,
        size_t p_size
    );
    int (*m_close)(struct ts_vfsFile *p_file);
    int (*m_llseek)(struct ts_vfsFile *p_file, loff_t p_offset, int p_whence);
};

struct ts_vfsNodeOperations;

struct ts_vfsNode {
    int m_referenceCount;
    struct ts_vfsNode *m_parent;
    char m_name[C_MAX_FILE_NAME_SIZE + 1];
    enum te_vfsNodeType m_type;

    union {
        dev_t m_device;
        struct ts_list *m_children;
    } m_specific;

    int m_uid;
    int m_gid;
    int m_mod;

    void *m_fileSystemData;

    const struct ts_vfsNodeOperations *m_operations;
};

struct ts_vfsNodeOperations {
    int (*m_lookup)(
        struct ts_vfsNode *p_node,
        const char *p_name,
        struct ts_vfsNode **p_result
    );
    int (*m_release)(struct ts_vfsNode *p_node);
    int (*m_open)(
        struct ts_vfsNode *p_node,
        int p_flags,
        struct ts_vfsFile *p_file
    );
    int (*m_mkdir)(
        struct ts_vfsNode *p_node,
        const char *p_name
    );
    int (*m_create)(
        struct ts_vfsNode *p_node,
        const char *p_name
    );
    int (*m_chmod)(
        struct ts_vfsNode *p_node,
        mode_t p_mode
    );
    int (*m_chown)(
        struct ts_vfsNode *p_node,
        uid_t p_owner
    );
    int (*m_chgrp)(
        struct ts_vfsNode *p_node,
        gid_t p_group
    );
    int (*m_rename)(
        struct ts_vfsNode *p_node,
        const char *p_name
    );
};

struct ts_vfsMountParameters {
    struct ts_vfsFile *m_nodeToMount;
};

struct ts_vfsFileSystem {
    const char *m_name;
    int (*m_mount)(
        struct ts_vfsNode *p_mountPoint,
        struct ts_vfsMountParameters *p_mountParameters
    );
    const struct ts_vfsNodeOperations *m_operations;
};

/**
 * @brief Initializes the VFS.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success.
 * @retval Any other value if an error occurred.
*/
int vfsInit(void);

/**
 * @brief Returns the VFS node for the given file.
 * 
 * @param[in] p_path The path to the file.
 * @param[out] p_result The pointer to the node corresponding to the requested
 * file.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the VFS node was found (p_result is modified)
 * @retval Any other value if the VFS node was not found (p_result is
 * unchanged).
 * 
 * @note When this function succeeds, the VFS node's reference counter is
 * incremented.
*/
int vfsLookup(const char *p_path, struct ts_vfsNode **p_result);

/**
 * @brief Returns the VFS node for the parent of the given file. This is
 * especially useful when creating files: you need to retrieve the parent of
 * the node that you want to create first, and then create its child. This
 * function will work even though the file does not exist.
 * 
 * @param[in] p_path The path of the file
 * @param[out] p_result The pointer to the node corresponding to the parent of
 * the requested file.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the VFS node was found (p_result is modified)
 * @retval Any other value if the VFS node was not found (p_result is
 * unchanged).
 * 
 * @note When this function succeeds, the VFS node's reference counter is
 * incremented.
*/
int vfsLookupParent(const char *p_path, struct ts_vfsNode **p_result);

/**
 * @brief Decreases the reference count of the given VFS node. If the reference
 * count reaches 0, the VFS node will effectively be freed.
 * 
 * @param[in] p_node The VFS node to release.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the node was released successfully.
 * @retval Any other value if an error occurred.
*/
int vfsRelease(struct ts_vfsNode *p_node);

/**
 * @brief Opens the given node.
 * 
 * @param[in] p_node The node to open.
 * @param[in] p_flags The open flags.
 * @param[out] p_file The opened file.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the file was opened successfully.
 * @retval Any other value if an error occurred.
 * 
 * @note This operation will increase the number of references to the VFS node.
*/
int vfsOpen(
    struct ts_vfsNode *p_node,
    int p_flags,
    struct ts_vfsFile **p_file
);

/**
 * @brief Closes the given file.
 * 
 * @param[in] p_file The file to close.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the file was closed successfully.
 * @retval Any other value if an error occurred.
 * 
 * @note This operation will decrease the number of references to the VFS node.
*/
int vfsFileClose(struct ts_vfsFile *p_file);

/**
 * @brief Reads at most p_size bytes from the given file into p_buffer.
 * 
 * @param[in] p_file The file to read the bytes from.
 * @param[out] p_buffer The buffer that will hold the data read.
 * @param[in] p_size The size of the buffer.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval < 0 if an error occurred.
 * @retval 0 if there are no more bytes to read from the file.
 * @retval > 0 indicates the number of bytes read into the buffer.
*/
ssize_t vfsFileRead(struct ts_vfsFile *p_file, void *p_buffer, size_t p_size);

/**
 * @brief Writes p_size bytes from the given buffer into the given file.
 * 
 * @param[in] p_file The file to write the bytes to.
 * @param[in] p_buffer The buffer that contains the data to write.
 * @param[in] p_size The size of the buffer.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval < 0 if an error occurred.
 * @retval >= 0 indicates the number of bytes written into the buffer.
*/
ssize_t vfsFileWrite(
    struct ts_vfsFile *p_file,
    const void *p_buffer,
    size_t p_size
);

/**
 * @brief Repositions the cursor in the given file.
 * 
 * @param[in] p_file The file to reposition the cursor in.
 * @param[in] p_offset The offset from the whence.
 * @param[in] p_whence The position from which to add/subtract the offset to get
 * the new position in the file.
*/
int vfsFileLlseek(struct ts_vfsFile *p_file, loff_t p_offset, int p_whence);

/**
 * @brief Registers the given file system.
 * 
 * @param[in] p_fileSystem The file system to register.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success.
 * @retval Any other value if an error occurred.
*/
int vfsRegisterFileSystem(const struct ts_vfsFileSystem *p_fileSystem);

/**
 * @brief Unregisters the given file system.
 * 
 * @param[in] p_fileSystem The file system to unregister.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success.
 * @retval Any other value if an error occurred.
*/
int vfsUnregisterFileSystem(const struct ts_vfsFileSystem *p_fileSystem);

/**
 * @brief Gets the given file system.
 * 
 * @param[in] p_name The name of the registered file system to retrieve.
 * 
 * @returns The registered file system with the given name.
 * @retval NULL if the given name does not correspond to any registered
 * file system.
*/
const struct ts_vfsFileSystem *vfsGetFileSystem(const char *p_name);

/**
 * @brief Creates the given directory.
 * 
 * @param[in] p_path The path to the new directory.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success.
 * @retval Any other value if an error occurred.
*/
int vfsMkdir(const char *p_path);

/**
 * @brief Creates the given file.
 * 
 * @param[in] p_path The path to the new file.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success.
 * @retval Any other value if an error occurred.
*/
int vfsCreate(const char *p_path);

/**
 * @brief Changes the mode of the given file.
 * 
 * @param[in] p_path The path to the file.
 * @param[in] p_mode The new mode of the file.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success.
 * @retval Any other value if an error occurred.
*/
int vfsChmod(const char *p_path, mode_t p_mode);

/**
 * @brief Changes the owner of the given file.
 * 
 * @param[in] p_path The path to the file.
 * @param[in] p_owner The new owner of the file.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success.
 * @retval Any other value if an error occurred.
*/
int vfsChown(const char *p_path, uid_t p_owner);

/**
 * @brief Changes the group of the given file.
 * 
 * @param[in] p_path The path to the file.
 * @param[in] p_group The new group of the file.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success.
 * @retval Any other value if an error occurred.
*/
int vfsChgrp(const char *p_path, gid_t p_group);

/**
 * @brief Renames the given file.
 * 
 * @param[in] p_path The path to the file.
 * @param[in] p_name The new name of the file.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success.
 * @retval Any other value if an error occurred.
*/
int vfsRename(const char *p_path, const char *p_name);

#endif
