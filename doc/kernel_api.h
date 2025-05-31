#include <stddef.h>
#include <stdint.h>

#include <sys/types.h>
#include "kernel_types.h"

#define C_KERNEL_OBJECT_NAME_SIZE 15

enum te_kernelObjectType {
    E_KERNELOBJECTTYPE_MOUNT = 1,
    E_KERNELOBJECTTYPE_BLOCK_DEVICE = 2,
    E_KERNELOBJECTTYPE_CHARACTER_DEVICE = 3,
    E_KERNELOBJECTTYPE_FILE = 4,
    E_KERNELOBJECTTYPE_DIRECTORY = 5,
    E_KERNELOBJECTTYPE_FILE_SYSTEM = 6,
    E_KERNELOBJECTTYPE_SYMLINK = 7,
    E_KERNELOBJECTTYPE_MODULE = 8,
    E_KERNELOBJECTTYPE_DEVICE = 9,
    E_KERNELOBJECTTYPE_FILE_DESCRIPTOR = 10
};

enum te_kernelErrorCode {
    E_KERNELERRORCODE_SUCCESS = 0,
    E_KERNELERRORCODE_INVALID_PARAMETER = -1,
    E_KERNELERRORCODE_NOT_FOUND = -2,
    E_KERNELERRORCODE_BUSY = -3,
    E_KERNELERRORCODE_NO_MEMORY = -4,
    E_KERNELERRORCODE_IO_ERROR = -5,
};

struct ts_kernelObject {
    enum te_kernelObjectType m_type;
    char m_name[C_KERNEL_OBJECT_NAME_SIZE + 1];
    struct ts_kernelObject *m_parent;
    struct ts_kernelObject *m_children;
    struct ts_kernelObject *m_next;
    struct ts_kernelObject *m_previous;
    unsigned int m_referenceCount;
    size_t m_size;
    void (*m_destroy)(struct ts_kernelObject *p_object);
    void *m_data;
};

struct ts_mount {
    struct ts_kernelObject m_object;
    struct ts_kernelObject *m_fileSystem;
    struct ts_kernelObject *m_directory;
};

struct ts_blockDevice {
    struct ts_kernelObject m_object;
    size_t m_blockSize;
    lba_t m_blockCount;
    int (*m_open)(
        struct ts_blockDevice *p_device,
        int flags,
        struct ts_fileDescriptor *p_fileDescriptor
    );
};

struct ts_characterDevice {
    struct ts_kernelObject m_object;
    int (*m_open)(
        struct ts_blockDevice *p_device,
        int flags,
        struct ts_fileDescriptor *p_fileDescriptor
    );
};

struct ts_file {
    struct ts_kernelObject m_object;
    mode_t m_mode;
    unsigned long m_size;
    uid_t m_uid;
    gid_t m_gid;
    time_t m_creationTime;
    time_t m_modificationTime;
    time_t m_accessTime;
    int (*m_open)(
        struct ts_blockDevice *p_device,
        int flags,
        struct ts_fileDescriptor *p_fileDescriptor
    );
};

struct ts_directory {
    struct ts_kernelObject m_object;
    mode_t m_mode;
    uid_t m_uid;
    gid_t m_gid;
    time_t m_creationTime;
    time_t m_modificationTime;
    time_t m_accessTime;
    int (*m_opendir)(
        struct ts_directory *p_directory,
        const char *p_name,
        struct ts_fileDescriptor *p_fileDescriptor
    );
};

struct ts_fileSystem {
    struct ts_kernelObject m_object;
    int (*m_mount)(
        struct ts_mount *p_mount,
        struct ts_fileSystem *p_fileSystem
    );
};

struct ts_symlink {
    struct ts_kernelObject m_object;
};

struct ts_module {
    struct ts_kernelObject m_object;
    int (*m_init)(struct ts_module *p_module);
    int (*m_exit)(struct ts_module *p_module);
};

struct ts_fileDescriptor {
    struct ts_kernelObject m_object;
    struct ts_kernelObject *m_file;
    off_t m_offset;
    int (*m_read)(
        struct ts_fileDescriptor *p_fileDescriptor,
        void *p_buffer,
        size_t p_size
    );
    int (*m_write)(
        struct ts_fileDescriptor *p_fileDescriptor,
        const void *p_buffer,
        size_t p_size
    );
    int (*m_ioctl)(
        struct ts_fileDescriptor *p_fileDescriptor,
        int request,
        void *arg
    );
    int (*m_close)(
        struct ts_fileDescriptor *p_fileDescriptor
    );
    int (*m_flush)(
        struct ts_fileDescriptor *p_fileDescriptor
    );
    int (*m_seek)(
        struct ts_fileDescriptor *p_fileDescriptor,
        off_t p_offset
    );
    int (*m_truncate)(
        struct ts_fileDescriptor *p_fileDescriptor,
        off_t p_length
    );
};

/**
 * @brief Gets a kernel object by its path.
 * @details If the kernel object is not found, p_object is not modified.
 * The reference count of the returned kernel object is incremented.
 * 
 * @param[in] p_baseObject The base object to use for relative paths. If NULL,
 * the path is considered absolute.
 * @param[in] p_path The path to the kernel object. A kernel object path can be
 * absolute or relative. Absolute paths start with a '/' character and are
 * relative to the root of the kernel object tree. Relative paths are relative
 * to p_baseObject.
 * @param[in] p_object A pointer to a pointer that will hold the address of the
 * kernel object.
 * 
 * @returns 0 on success, or a negative error code on failure.
 */
int kernelObjectGet(
    struct ts_kernelObject *p_baseObject,
    const char *p_path,
    struct ts_kernelObject **p_object
);

/**
 * @brief Decrements the reference count of a kernel object.
 * @details If the reference count reaches zero, the kernel object is
 * destroyed.
 * 
 * @param[in] p_object The kernel object to decrement the reference count of.
 * 
 * @returns 0 on success, or a negative error code on failure.
 */
int kernelObjectRelease(struct ts_kernelObject *p_object);

/**
 * @brief Adds a child kernel object to a parent kernel object.
 * @details The child kernel object is added to the parent's list of children.
 * The reference count of the parent kernel object is incremented.
 * 
 * @param[in] p_parent The parent kernel object.
 * @param[in] p_child The child kernel object to add.
 * 
 * @returns 0 on success, or a negative error code on failure.
 * @note The child kernel object must not already have a parent.
 */
int kernelObjectAddChild(
    struct ts_kernelObject *p_parent,
    struct ts_kernelObject *p_child
);

/**
 * @brief Removes a child kernel object from a parent kernel object.
 * @details The child kernel object is removed from the parent's list of
 * children. The reference count of the parent kernel object is decremented.
 * 
 * @param[in] p_parent The parent kernel object.
 * @param[in] p_child The child kernel object to remove.
 * 
 * @returns 0 on success, or a negative error code on failure.
 */
int kernelObjectRemoveChild(
    struct ts_kernelObject *p_parent,
    struct ts_kernelObject *p_child
);

/**
 * @brief Gets the root kernel object.
 * @details The root kernel object is the top-level kernel object in the
 * kernel object tree. The reference count of the root kernel object is
 * incremented.
 * 
 * @param[out] p_root A pointer to a pointer that will hold the address of the
 * root kernel object.
 * 
 * @returns 0 on success, or a negative error code on failure.
 */
int kernelObjectGetRoot(struct ts_kernelObject **p_root);
