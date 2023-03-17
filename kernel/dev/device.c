#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <kernel/dev/device.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/misc/spinlock.h>

#define C_DEVICE_MAX_MAJOR 255
#define C_DEVICE_MAX_MINOR 255

struct ts_deviceMinorEntry {
    bool a_taken;
    const struct ts_vfsNodeOperations *a_operations;
};

struct ts_deviceMajorEntry {
    enum te_deviceType a_type;
    const char *a_name;
    struct ts_deviceMinorEntry a_entries[C_DEVICE_MAX_MINOR + 1];
};

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
static int deviceGetFreeMajor(void);

static struct ts_deviceMajorEntry *s_deviceTable[C_DEVICE_MAX_MAJOR + 1];
static t_spinlock s_deviceTableSpinlock;

int deviceInit(void) {
    spinlockInit(&s_deviceTableSpinlock);
    memset(s_deviceTable, 0, sizeof(s_deviceTable));

    return 0;
}

int deviceRegister(
    enum te_deviceType p_type,
    const char *p_name,
    dev_t *p_deviceNumber,
    int p_minorCount
) {
    int l_major = deviceGetMajor(*p_deviceNumber);
    int l_minor = deviceGetMinor(*p_deviceNumber);

    // Make sure that the major and minor numbers are valid
    if(
        l_major < 0
        || l_major > C_DEVICE_MAX_MAJOR
        || l_minor < 0
        || l_minor > C_DEVICE_MAX_MINOR
    ) {
        return -1;
    }

    // Make sure that the minor count is valid
    if((p_minorCount < 0) || (p_minorCount >= (C_DEVICE_MAX_MINOR + 1))) {
        return -1;
    }

    // Make sure that the device range is valid
    if((l_minor + p_minorCount) > (C_DEVICE_MAX_MINOR + 1)) {
        return -1;
    }

    spinlockAcquire(&s_deviceTableSpinlock);

    if(deviceGetMajor(*p_deviceNumber) == 0) {
        // If the major number is 0, allocate a free major number.
        l_major = deviceGetFreeMajor();

        if(l_major < 0) {
            spinlockRelease(&s_deviceTableSpinlock);
            return -1;
        }

        deviceSetMajor(p_deviceNumber, l_major);
    } else if(s_deviceTable[l_major] != NULL) {
        // If the major number already exists, make sure that the device driver
        // name is the same.
        if(strcmp(p_name, s_deviceTable[l_major]->a_name) != 0) {
            spinlockRelease(&s_deviceTableSpinlock);
            return -1;
        }

        // Make sure that the minor numbers are free.
        int l_currentMinor = l_minor;

        for(int l_index = 0; l_index < p_minorCount; l_index++) {
            if(s_deviceTable[l_major]->a_entries[l_currentMinor].a_taken) {
                spinlockRelease(&s_deviceTableSpinlock);
                return -1;
            }

            l_currentMinor++;
        }
    }

    if(s_deviceTable[l_major] == NULL) {
        // If the major number does not exist yet, allocate it.
        s_deviceTable[l_major] = kcalloc(sizeof(struct ts_deviceMajorEntry));

        if(s_deviceTable[l_major] == NULL) {
            spinlockRelease(&s_deviceTableSpinlock);
            return -1;
        }

        s_deviceTable[l_major]->a_type = p_type;
        s_deviceTable[l_major]->a_name = p_name;
    }

    // Allocate the minor devices
    int l_currentMinor = l_minor;

    for(int l_index = 0; l_index < p_minorCount; l_index++) {
        s_deviceTable[l_major]->a_entries[l_currentMinor].a_taken = true;
        l_currentMinor++;
    }

    spinlockRelease(&s_deviceTableSpinlock);

    return 0;
}

int deviceAdd(
    const char *p_name,
    dev_t p_deviceNumber,
    const struct ts_vfsNodeOperations *p_operations,
    int p_minorCount
) {
    int l_major = deviceGetMajor(p_deviceNumber);
    int l_minor = deviceGetMinor(p_deviceNumber);

    // Make sure that the major and minor numbers are valid
    if(
        l_major < 1
        || l_major > C_DEVICE_MAX_MAJOR
        || l_minor < 0
        || l_minor > C_DEVICE_MAX_MINOR
    ) {
        return -1;
    }

    // Make sure that the minor count is valid
    if((p_minorCount < 0) || (p_minorCount >= (C_DEVICE_MAX_MINOR + 1))) {
        return -1;
    }

    // Make sure that the device range is valid
    if((l_minor + p_minorCount) > (C_DEVICE_MAX_MINOR + 1)) {
        return -1;
    }

    spinlockAcquire(&s_deviceTableSpinlock);

    // Make sure that the device driver name is correct
    if(strcmp(p_name, s_deviceTable[l_major]->a_name) != 0) {
        spinlockRelease(&s_deviceTableSpinlock);
        return -1;
    }

    // Set the device operations
    int l_currentMinor = l_minor;

    for(int l_index = 0; l_index < p_minorCount; l_index++) {
        s_deviceTable[l_major]->a_entries[l_currentMinor].a_operations =
            p_operations;
        l_currentMinor++;
    }

    spinlockRelease(&s_deviceTableSpinlock);

    return 0;
}

int deviceGetOperations(
    dev_t p_deviceNumber,
    const struct ts_vfsNodeOperations **l_output
) {
    int l_major = deviceGetMajor(p_deviceNumber);
    int l_minor = deviceGetMinor(p_deviceNumber);

    spinlockAcquire(&s_deviceTableSpinlock);

    if(s_deviceTable[l_major] == NULL) {
        spinlockRelease(&s_deviceTableSpinlock);
        return -ENODEV;
    }

    if(!s_deviceTable[l_major]->a_entries[l_minor].a_taken) {
        spinlockRelease(&s_deviceTableSpinlock);
        return -ENODEV;
    }

    *l_output = s_deviceTable[l_major]->a_entries[l_minor].a_operations;

    spinlockRelease(&s_deviceTableSpinlock);

    return 0;
}

static int deviceGetFreeMajor(void) {
    for(int l_major = 1; l_major <= C_DEVICE_MAX_MAJOR; l_major++) {
        if(s_deviceTable[l_major] == NULL) {
            return l_major;
        }
    }

    return -1;
}
