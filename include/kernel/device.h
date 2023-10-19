#ifndef __INCLUDE_KERNEL_DEVICE_H__
#define __INCLUDE_KERNEL_DEVICE_H__

#include "kernel/fs/vfs.h"

#define M_DEVICE_MAJOR(p_device) (p_device >> 8)
#define M_DEVICE_MINOR(p_device) (p_device & 0xff)
#define M_DEVICE_MAKE(p_major, p_minor) ((p_major << 8) | p_minor)

struct ts_deviceInformation {
    const char *m_name;
    const struct ts_vfsFileOperations *m_fileOperations;
};

/**
 * @brief Initializes the device system.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success.
 * @retval Any other value on error.
*/
int deviceInit(void);

/**
 * @brief Registers a character device driver.
 * 
 * @param[in] p_major The requested major number. If this number is 0, then a
 * new allocated major number will be returned.
 * @param[in] p_name The name of the device driver.
 * @param[in] p_fileOperations A pointer to the file operations structure used
 * by the device files.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the requested major device number was successfully allocated.
 * @retval p_major if a new major number was successfully allocated.
 * @retval -EBUSY if the requested major number is already taken or if there are
 * no more major device numbers to allocate.
 * @retval -EINVAL if p_major exceeds bounds, if p_name is NULL or if
 * p_fileOperations is NULL.
*/
int deviceCharacterRegister(
    unsigned int p_major,
    const char *p_name,
    const struct ts_vfsFileOperations *p_fileOperations
);

/**
 * @brief Unregisters the given device major number.
 * 
 * @param[in] p_major The major device number to unregister.
*/
void deviceCharacterUnregister(
    unsigned int p_major
);

/**
 * @brief Returns information about the device with the given major number.
 * 
 * @param[in] p_major The major number of the device.
 * 
 * @returns A pointer to the device information structure.
 * @retval NULL if the major number is invalid or not allocated.
 * 
 * @note The returned pointer must not be freed!
*/
const struct ts_deviceInformation *deviceCharacterGetInformation(
    unsigned int p_major
);

#endif
