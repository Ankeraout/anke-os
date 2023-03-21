#include <errno.h>
#include <kernel/common.h>
#include <kernel/debug.h>
#include <kernel/dev/device.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <kernel/module.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int mbrfsInit(const char *p_args);
static void mbrfsQuit(void);

struct ts_mbrfsPartitionEntry {
    uint8_t a_bootIndicatorFlag;
    uint8_t a_startingHead;
    uint16_t a_startingSector : 6;
    uint16_t a_startingCylinder : 10;
    uint8_t a_systemId;
    uint8_t a_endingHead;
    uint16_t a_endingSector : 6;
    uint16_t a_endingCylinder : 10;
    uint32_t a_startingLba;
    uint32_t a_totalSectors;
} __attribute__((packed));

struct ts_mbrfsMbr {
    uint8_t a_code[446];
    struct ts_mbrfsPartitionEntry a_partitions[4];
    uint16_t a_signature;
} __attribute__((packed));

struct ts_mbrfsData {
    struct ts_vfsNode *a_drive;
    uint64_t a_lbaPartitionStart;
    uint64_t a_partitionSizeSectors;
};

static int mbrfsOnMount(const char *p_file, const char *p_mountPoint);
static ssize_t mbrfsOperationRead(
    struct ts_vfsNode *p_node,
    off_t p_offset,
    void *p_buffer,
    size_t p_size
);
static ssize_t mbrfsOperationWrite(
    struct ts_vfsNode *p_node,
    off_t p_offset,
    const void *p_buffer,
    size_t p_size
);

M_DECLARE_MODULE const struct ts_module g_moduleMbrfs = {
    .a_name = "mbrfs",
    .a_init = mbrfsInit,
    .a_quit = mbrfsQuit
};

static const struct ts_vfsNodeOperations s_mbrfsOperations = {
    .a_read = mbrfsOperationRead,
    .a_write = mbrfsOperationWrite
};

static const struct ts_vfsFileSystem s_mbrfsFileSystem = {
    .a_name = "mbrfs",
    .a_onMount = mbrfsOnMount,
    .a_operations = &s_mbrfsOperations
};

static int mbrfsInit(const char *p_args) {
    M_UNUSED_PARAMETER(p_args);

    return vfsRegisterFileSystem(&s_mbrfsFileSystem);
}

static void mbrfsQuit(void) {

}

static int mbrfsOnMount(const char *p_file, const char *p_mountPoint) {
    M_UNUSED_PARAMETER(p_file);
    M_UNUSED_PARAMETER(p_mountPoint);

    dev_t l_deviceNumber = deviceMake(0, 0);
    const char *l_fileName = strchr(p_file, '/') + 1;
    char *l_driverName = kmalloc(NAME_MAX + 1);

    if(l_driverName == NULL) {
        debug("mbrfs: Failed to allocate memory for driver name.\n");
        return -ENOMEM;
    }

    snprintf(l_driverName, NAME_MAX, "%s-", l_fileName);

    int l_returnValue = deviceRegister(
        E_DEVICETYPE_BLOCK,
        l_driverName,
        &l_deviceNumber,
        4
    );

    if(l_returnValue != 0) {
        debug(
            "mbrfs: Failed to register device %s: %d.\n",
            l_driverName,
            l_returnValue
        );

        return l_returnValue;
    }

    debug("mbrfs: mount(\"%s\")\n", p_file);

    // Open file
    struct ts_vfsNode *l_file;
    l_returnValue = vfsLookup(NULL, p_file, &l_file);

    if(l_returnValue != 0) {
        debug("mbrfs: Failed to open %s.\n", p_file);
        return l_returnValue;
    }

    // Read MBR
    struct ts_mbrfsMbr l_mbr;

    ssize_t l_readBytes = vfsOperationRead(l_file, 0, &l_mbr, 512);

    if(l_readBytes != 512) {
        debug("mbrfs: Failed to read MBR: %d.\n", l_mbr);
        vfsOperationClose(l_file);
        return l_returnValue;
    }

    // Scan partition table
    for(int l_index = 0; l_index < 4; l_index++) {
        // If all bytes are set to 0, then the partition is unused.
        bool l_unused = true;

        for(int l_index2 = 0; l_index2 < 16; l_index2++) {
            if(((uint8_t *)&l_mbr.a_partitions[l_index])[l_index2] != 0) {
                l_unused = false;
                break;
            }
        }

        if(l_unused) {
            continue;
        }

        // Allocate and initialize data structure
        struct ts_mbrfsData *l_partitionData =
            kmalloc(sizeof(struct ts_mbrfsData));

        if(l_partitionData == NULL) {
            debug("mbfs: Failed to allocate memory for disk partition data.\n");
            return -ENOMEM;
        }

        l_partitionData->a_drive = l_file;
        l_partitionData->a_lbaPartitionStart =
            l_mbr.a_partitions[l_index].a_startingLba;
        l_partitionData->a_partitionSizeSectors =
            l_mbr.a_partitions[l_index].a_totalSectors;

        // Create partition device
        deviceSetMinor(&l_deviceNumber, l_index);

        l_returnValue =
            deviceAdd(l_driverName, l_deviceNumber, &s_mbrfsOperations, 1);

        if(l_returnValue != 0) {
            debug("mbrfs: Failed to add disk partition device.\n", p_file);
            return l_returnValue;
        }

        // Set device data
        deviceSetData(l_deviceNumber, l_partitionData);

        // Create partition device file
        l_returnValue = deviceCreateFile(l_deviceNumber);

        if(l_returnValue != 0) {
            debug("mbrfs: Failed to create disk partition device file.\n");
            return l_returnValue;
        }

        debug("mbrfs: Created partition file for partition %d.\n", l_index);
    }

    return 0;
}

static ssize_t mbrfsOperationRead(
    struct ts_vfsNode *p_node,
    off_t p_offset,
    void *p_buffer,
    size_t p_size
) {
    struct ts_mbrfsData *l_data;

    deviceGetData(p_node->a_deviceNumber, (void **)&l_data);

    return vfsOperationRead(
        l_data->a_drive,
        p_offset + l_data->a_lbaPartitionStart * 512,
        p_buffer,
        p_size
    );
}

static ssize_t mbrfsOperationWrite(
    struct ts_vfsNode *p_node,
    off_t p_offset,
    const void *p_buffer,
    size_t p_size
) {
    struct ts_mbrfsData *l_data;

    deviceGetData(p_node->a_deviceNumber, (void **)&l_data);

    return vfsOperationWrite(
        l_data->a_drive,
        p_offset + l_data->a_lbaPartitionStart * 512,
        p_buffer,
        p_size
    );
}
