#ifndef __INCLUDE_KERNEL_DEV_DEVICE_H__
#define __INCLUDE_KERNEL_DEV_DEVICE_H__

#include <kernel/fs/vfs.h>
#include <stdint.h>
#include <sys/types.h>

enum te_deviceType {
    E_DEVICETYPE_CHARACTER,
    E_DEVICETYPE_BLOCK
};

/**
 * @brief Returns the major number of the given device number.
 */
static inline uint8_t deviceGetMajor(dev_t p_deviceNumber) {
    return p_deviceNumber >> 8;
}

/**
 * @brief Returns the minor number of the given device number.
 */
static inline uint8_t deviceGetMinor(dev_t p_deviceNumber) {
    return p_deviceNumber & 0xff;
}

/**
 * @brief Sets the major number of the given device number.
 */
static inline void deviceSetMajor(dev_t *p_deviceNumber, uint8_t p_major) {
    *p_deviceNumber = ((*p_deviceNumber) & 0x00ff) | (p_major << 8);
}

/**
 * @brief Sets the minor number of the given device number.
 */
static inline void deviceSetMinor(dev_t *p_deviceNumber, uint8_t p_minor) {
    *p_deviceNumber = ((*p_deviceNumber) & 0xff00) | p_minor;
}

/**
 * @brief Returns the dev_t value for the corresponding major and minor device
 * numbers.
*/
static inline dev_t deviceMake(uint8_t p_major, uint8_t p_minor) {
    return (p_major << 8) | p_minor;
}

/**
 * @brief Initializes the device system.
 *
 * @retval 0 on success.
 * @retval Any other (negative) value if an error occurred.
 */
int deviceInit(void);

/**
 * @brief Allocates device numbers.
 *
 * @param[in] p_type The type of the device.
 * @param[in] p_name The name of the device driver.
 * @param[in, out] p_deviceNumber The number of the first device. If the major
 * number is set to 0, then a new major number will be allocated and returned
 * in the structure. The minor number is fixed.
 * @param[in] p_minorCount The number of minor devices to reserve.
 *
 * @returns An integer value that indicates the result of the operation.
 * @retval 0 on success
 * @retval Any other value on error.
 */
int deviceRegister(
    enum te_deviceType p_type,
    const char *p_name,
    dev_t *p_deviceNumber,
    int p_minorCount
);

/**
 * @brief Adds a device to the system.
 *
 * @param[in] p_name The name of the device driver.
 * @param[in] p_deviceNumber The number of the first device.
 * @param[in] p_operations The operations for the device.
 * @param[in] p_minorCount The number of (consecutive) minor devices to add.
 *
 * @returns An integer value that indicates the result of the operation.
 * @retval 0 on success
 * @retval Any other value on error.
 */
int deviceAdd(
    const char *p_name,
    dev_t p_deviceNumber,
    const struct ts_vfsNodeOperations *p_operations,
    int p_minorCount
);

/**
 * @brief Gets the operation for the given device.
 *
 * @param[out] l_output The ts_vfsNodeOperations structure pointer that will
 * receive a pointer to the device operations. Note that the output pointer
 * must not be freed!
 *
 * @returns An integer value that indicates the result of the operation.
 * @retval 0 on success
 * @retval Any other value on error.
 */
int deviceGetOperations(
    dev_t p_deviceNumber,
    const struct ts_vfsNodeOperations **l_output
);

#endif
