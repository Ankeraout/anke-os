#include "kernel/device.h"
#include "kernel/fs/vfs.h"
#include "klibc/string.h"

#define C_DEVICE_COUNT 256

static struct ts_deviceInformation s_characterDevices[C_DEVICE_COUNT];

int deviceInit(void) {
    kmemset(s_characterDevices, 0, sizeof(s_characterDevices));
    return 0;
}

int deviceCharacterRegister(
    unsigned int p_major,
    const char *p_name,
    const struct ts_vfsFileOperations *p_fileOperations
) {
    if(
        (p_name == NULL)
        || (p_fileOperations == NULL)
        || (p_major >= C_DEVICE_COUNT)
    ) {
        return -EINVAL;
    }

    int l_major;

    if(p_major == 0) {
        l_major = 1;

        while(l_major < C_DEVICE_COUNT) {
            if(s_characterDevices[l_major].m_name == NULL) {
                break;
            }

            l_major++;
        }

        if(l_major == C_DEVICE_COUNT) {
            return -EBUSY;
        }
    } else if(s_characterDevices[p_major].m_name != NULL) {
        return -EBUSY;
    }

    s_characterDevices[p_major].m_name = p_name;
    s_characterDevices[p_major].m_fileOperations = p_fileOperations;

    if(p_major == 0) {
        return l_major;
    } else {
        return 0;
    }
}

void deviceCharacterUnregister(
    unsigned int p_major
) {
    if((p_major == 0) || (p_major >= C_DEVICE_COUNT)) {
        return;
    }

    s_characterDevices[p_major].m_name = NULL;
    s_characterDevices[p_major].m_fileOperations = NULL;
}

const struct ts_deviceInformation *deviceCharacterGetInformation(
    unsigned int p_major
) {
    if(
        (p_major == 0)
        || (p_major >= C_DEVICE_COUNT)
        || (s_characterDevices[p_major].m_name == NULL)
    ) {
        return NULL;
    }

    return &s_characterDevices[p_major];
}
