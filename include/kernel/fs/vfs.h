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
    loff_t m_offset;
    struct ts_vfsFileOperations *m_operations;
    int m_flags;
};

struct ts_vfsFileOperations {
    ssize_t (*m_read)(
        struct ts_vfsFile *p_file,
        void *p_buffer,
        size_t p_size,
        loff_t p_offset
    );
    ssize_t (*m_write)(
        struct ts_vfsFile *p_file,
        const void *p_buffer,
        size_t p_size,
        loff_t p_offset
    );
    int (*m_close)(struct ts_vfsFile *p_file);
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

    struct ts_vfsNodeOperations *m_operations;
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
};

/**
 * @brief Initializes the VFS.
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
int vfsClose(struct ts_vfsFile *p_file);

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
ssize_t vfsRead(struct ts_vfsFile *p_file, void *p_buffer, size_t p_size);

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
ssize_t vfsWrite(
    struct ts_vfsFile *p_file,
    const void *p_buffer,
    size_t p_size
);

#endif
