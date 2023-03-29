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

#define C_FATFS_ATTRIBUTE_DIRECTORY 0x10

enum te_fatfsType {
    E_FATFSTYPE_FAT12,
    E_FATFSTYPE_FAT16,
    E_FATFSTYPE_FAT32,
    E_FATFSTYPE_EXFAT
};

struct ts_fatfsBootRecordCommon {
    uint8_t a_jumpInstruction[3];
    uint8_t a_oemIdentifier[8];
    uint16_t a_bytesPerSector;
    uint8_t a_sectorsPerCluster;
    uint16_t a_reservedSectors;
    uint8_t a_fatCount;
    uint16_t a_rootDirectoryEntryCount;
    uint16_t a_totalSectors;
    uint8_t a_mediaDescriptorType;
    uint16_t a_sectorsPerFat;
    uint16_t a_sectorsPerTrack;
    uint16_t a_headCount;
    uint32_t a_hiddenSectorCount;
    uint32_t a_largeSectorCount;
} __attribute__((packed));

struct ts_fatfsBootRecord16 {
    struct ts_fatfsBootRecordCommon a_common;
    uint8_t a_driveNumber;
    uint8_t a_flags;
    uint8_t a_signature;
    uint32_t a_serial;
    uint8_t a_label[11];
    uint8_t a_systemIdentifier[8];
    uint8_t a_bootCode[448];
    uint16_t a_signature2;
} __attribute__((packed));

struct ts_fatfsInfo {
    struct ts_vfsNode *a_drive;
    enum te_fatfsType a_type;
    size_t a_bytesPerSector;
    size_t a_sectorsPerCluster;
    size_t a_bytesPerCluster;
    size_t a_fatFirstSector;
    size_t a_fatFirstByte;
    size_t a_sectorsPerFat;
    size_t a_bytesPerFat;
    size_t a_fatCount;
    size_t a_rootFirstSector;
    size_t a_rootFirstByte;
    size_t a_rootSectorCount;
    size_t a_rootEntryCount;
    size_t a_rootByteCount;
    size_t a_clusterFirstSector;
    size_t a_clusterFirstByte;
    size_t a_clusterCount;
};

struct ts_fatfsFileInfo {
    const struct ts_fatfsInfo *a_info;
    uint32_t a_cluster;
    uint32_t a_size;
};

struct ts_fatfsTime {
    uint16_t a_creationTimeHour : 5;
    uint16_t a_creationTimeMinutes : 6;
    uint16_t a_creationTimeSeconds : 5;
} __attribute__((packed));

struct ts_fatfsDate {
    uint16_t a_creationTimeYear : 7;
    uint16_t a_creationTimeMonth : 4;
    uint16_t a_creationTimeDay : 5;
} __attribute__((packed));

struct ts_fatfsDirectoryEntry {
    uint8_t a_name[8];
    uint8_t a_extension[3];
    uint8_t a_attributes;
    uint8_t a_reserved;
    uint8_t a_creationTimeDeciseconds;
    struct ts_fatfsTime a_creationTime;
    struct ts_fatfsDate a_creationDate;
    struct ts_fatfsDate a_accessDate;
    uint16_t a_clusterHigh;
    struct ts_fatfsTime a_modificationTime;
    struct ts_fatfsDate a_modificationDate;
    uint16_t a_clusterLow;
    uint32_t a_fileSize;
} __attribute__((packed));

static char *fatfsGetName(
    const struct ts_fatfsDirectoryEntry *p_directoryEntry,
    char *p_output
);
static int fatfsInit(const char *p_args);
static int fatfsOperationLookupDirectory(
    struct ts_vfsNode *p_node,
    const char *p_name,
    struct ts_vfsNode **p_output
);
static int fatfsOperationLookupFile(
    struct ts_vfsNode *p_node,
    const char *p_name,
    struct ts_vfsNode **p_output
);
static int fatfsOperationLookupFound(
    struct ts_vfsNode *p_node,
    const char *p_fileName,
    const struct ts_fatfsDirectoryEntry *p_entry,
    struct ts_vfsNode **p_output
);
static int fatfsOperationLookupRoot(
    struct ts_vfsNode *p_node,
    const char *p_name,
    struct ts_vfsNode **p_output
);
static int fatfsOnMount(const char *p_file, const char *p_mountPoint);
static int fatfsReadCluster(
    const struct ts_fatfsInfo *l_info,
    size_t l_clusterIndex,
    void *p_buffer,
    size_t p_size
);
static int64_t fatfsReadFatEntry(
    const struct ts_fatfsInfo *l_info,
    size_t l_entryIndex
);
static int64_t fatfsReadFatEntry16(
    const struct ts_fatfsInfo *l_info,
    size_t l_entryIndex
);
static void fatfsQuit(void);

M_DECLARE_MODULE const struct ts_module g_moduleFatfs = {
    .a_name = "fatfs",
    .a_init = fatfsInit,
    .a_quit = fatfsQuit
};

static const struct ts_vfsNodeOperations s_fatfsOperationsRoot = {
    .a_lookup = fatfsOperationLookupRoot
};

static const struct ts_vfsNodeOperations s_fatfsOperationsDirectory = {
    .a_lookup = fatfsOperationLookupDirectory
};

static const struct ts_vfsNodeOperations s_fatfsOperationsFile = {
    .a_lookup = fatfsOperationLookupFile
};

static const struct ts_vfsFileSystem s_fatfsFileSystem = {
    .a_name = "fatfs",
    .a_onMount = fatfsOnMount
};

static char *fatfsGetName(
    const struct ts_fatfsDirectoryEntry *p_directoryEntry,
    char *p_output
) {
    int l_fileNameLength = 0;

    for(int l_index = 0; l_index < 8; l_index++) {
        if(p_directoryEntry->a_name[l_index] != ' ') {
            l_fileNameLength = l_index + 1;
        }
    }

    int l_extensionLength = 0;

    for(int l_index = 0; l_index < 3; l_index++) {
        if(p_directoryEntry->a_name[l_index + 8] != ' ') {
            l_extensionLength = l_index + 1;
        }
    }

    memcpy(p_output, p_directoryEntry->a_name, l_fileNameLength);

    if(l_extensionLength > 0) {
        p_output[l_fileNameLength] = '.';
        memcpy(
            &p_output[l_fileNameLength + 1],
            &p_directoryEntry->a_name[8],
            l_extensionLength
        );
        p_output[l_fileNameLength + 1 + l_extensionLength] = '\0';
    } else {
        p_output[l_fileNameLength] = '\0';
    }

    return p_output;
}

static int fatfsInit(const char *p_args) {
    M_UNUSED_PARAMETER(p_args);

    return vfsRegisterFileSystem(&s_fatfsFileSystem);
}

static int fatfsOperationLookupDirectory(
    struct ts_vfsNode *p_node,
    const char *p_name,
    struct ts_vfsNode **p_output
) {
    // Get the node data
    struct ts_fatfsFileInfo *l_info = p_node->a_fsData;

    // Create a buffer for the cluster data.
    size_t l_entriesPerCluster = l_info->a_info->a_bytesPerCluster /
        sizeof(struct ts_fatfsDirectoryEntry);
    struct ts_fatfsDirectoryEntry l_buffer[l_entriesPerCluster];

    // Compute entry count
    size_t l_entryCount = l_info->a_size /
        sizeof(struct ts_fatfsDirectoryEntry);

    // Iterate through the directory entries
    size_t l_entryIndex = 0;
    uint32_t l_currentCluster = l_info->a_cluster;

    while(l_entryIndex < l_entryCount) {
        // Read the current cluster
        int l_returnValue = fatfsReadCluster(
            l_info->a_info,
            l_currentCluster,
            l_buffer,
            l_info->a_info->a_bytesPerCluster
        );

        if(l_returnValue < 0) {
            return l_returnValue;
        }

        // Iterate through the cluster entries
        size_t l_clusterEntryIndex = 0;

        do {
            // Get the file name
            char l_fileName[13];

            fatfsGetName(&l_buffer[l_clusterEntryIndex], l_fileName);

            // If the file name corresponds
            if(strcmp(l_fileName, p_name) == 0) {
                return fatfsOperationLookupFound(
                    p_node,
                    l_fileName,
                    &l_buffer[l_clusterEntryIndex],
                    p_output
                );
            }


            l_clusterEntryIndex++;
        } while(l_clusterEntryIndex < l_entriesPerCluster);

        // Get the next cluster number
        l_currentCluster = fatfsReadFatEntry(l_info->a_info, l_currentCluster);
    }

    return -ENOENT;
}

static int fatfsOperationLookupFile(
    struct ts_vfsNode *p_node,
    const char *p_name,
    struct ts_vfsNode **p_output
) {
    M_UNUSED_PARAMETER(p_node);
    M_UNUSED_PARAMETER(p_name);
    M_UNUSED_PARAMETER(p_output);

    // We cannot lookup in a file.

    return -ENOTDIR;
}

static int fatfsOperationLookupFound(
    struct ts_vfsNode *p_node,
    const char *p_fileName,
    const struct ts_fatfsDirectoryEntry *p_entry,
    struct ts_vfsNode **p_output
) {
    // Get the node data
    struct ts_fatfsFileInfo *l_info = p_node->a_fsData;

    // Allocate memory for the information structure
    struct ts_fatfsFileInfo *l_fileInfo =
        kmalloc(sizeof(struct ts_fatfsFileInfo));

    if(l_fileInfo == NULL) {
        return -ENOMEM;
    }

    // Create a new VFS node
    struct ts_vfsNode *l_file;
    int l_returnValue =
        vfsCreateNode(p_node, p_fileName, true, &l_file);

    if(l_returnValue != 0) {
        kfree(l_fileInfo);
        return l_returnValue;
    }

    // Prepare the file information structure
    l_fileInfo->a_info = l_info->a_info;
    l_fileInfo->a_cluster =
        (p_entry->a_clusterHigh << 16) | p_entry->a_clusterLow;
    l_fileInfo->a_size = p_entry->a_fileSize;

    // Prepare the VFS node
    l_file->a_fsData = l_fileInfo;

    if((p_entry->a_attributes & C_FATFS_ATTRIBUTE_DIRECTORY) != 0) {
        l_file->a_type = E_VFSNODETYPE_DIRECTORY;
        l_file->a_operations = &s_fatfsOperationsDirectory;
    } else {
        l_file->a_type = E_VFSNODETYPE_FILE;
        l_file->a_operations = &s_fatfsOperationsFile;
    }

    // Return the new node
    *p_output = l_file;

    return 0;
}

static int fatfsOperationLookupRoot(
    struct ts_vfsNode *p_node,
    const char *p_name,
    struct ts_vfsNode **p_output
) {
    // Get the node data
    struct ts_fatfsFileInfo *l_info = p_node->a_fsData;

    // Compute the disk offset
    off_t l_offset = l_info->a_info->a_rootFirstByte;
    off_t l_rootEnd = l_offset + l_info->a_info->a_rootByteCount;

    while(l_offset < l_rootEnd) {
        struct ts_fatfsDirectoryEntry l_entry;

        // Read entry from the disk
        ssize_t l_bytesRead = vfsOperationRead(
            l_info->a_info->a_drive,
            l_offset,
            &l_entry,
            sizeof(struct ts_fatfsDirectoryEntry)
        );

        // If an error occurred, return the error.
        if(l_bytesRead < 0) {
            return l_bytesRead;
        }

        // Get the file name
        char l_fileName[13];

        fatfsGetName(&l_entry, l_fileName);

        // If the file name corresponds
        if(strcmp(l_fileName, p_name) == 0) {
            return fatfsOperationLookupFound(
                p_node,
                l_fileName,
                &l_entry,
                p_output
            );
        }

        l_offset += sizeof(struct ts_fatfsDirectoryEntry);
    }

    return -ENOENT;
}

static void fatfsQuit(void) {

}

static int fatfsOnMount(const char *p_file, const char *p_mountPoint) {
    debug("fatfs: mount(\"%s\", \"%s\")\n", p_file, p_mountPoint);

    // Get the mount point node
    struct ts_vfsNode *l_mountPoint;
    int l_returnValue = vfsLookup(NULL, p_mountPoint, &l_mountPoint);

    if(l_returnValue != 0) {
        debug("fatfs: Failed to open %s: %d.\n", p_mountPoint, l_returnValue);
        return l_returnValue;
    }

    // The mount point must be a directory
    if(l_mountPoint->a_type != E_VFSNODETYPE_DIRECTORY) {
        debug("fatfs: %s is not a directory.\n", p_mountPoint);
        return -ENOTDIR;
    }

    // Get the file node
    struct ts_vfsNode *l_file;
    l_returnValue = vfsLookup(NULL, p_file, &l_file);

    if(l_returnValue != 0) {
        debug("fatfs: Failed to open %s.\n", p_file);
        vfsOperationClose(l_mountPoint);
        return l_returnValue;
    }

    // Read the VBR
    uint8_t l_vbr[512];
    ssize_t l_bytesRead = vfsOperationRead(l_file, 0, l_vbr, 512);

    if(l_bytesRead < 0) {
        debug("fatfs: Failed to read VBR: %d.\n", l_bytesRead);
        return l_bytesRead;
    }

    // Detect FAT type
    struct ts_fatfsBootRecordCommon *l_bootRecord =
        (struct ts_fatfsBootRecordCommon *)l_vbr;

    // Detect ExFAT
    if(l_bootRecord->a_bytesPerSector == 0) {
        debug("fatfs: Unsupported ExFAT filesystem.\n");
        vfsOperationClose(l_mountPoint);
        vfsOperationClose(l_file);
        return 1;
    }

    // Allocate memory for FAT info structure
    struct ts_fatfsInfo *l_info = kmalloc(sizeof(struct ts_fatfsInfo));

    if(l_info == NULL) {
        debug("fatfs: Failed to allocate memory for fatfs structure.\n");
        vfsOperationClose(l_mountPoint);
        vfsOperationClose(l_file);
        return -ENOMEM;
    }

    // Initialize info structure
    l_info->a_drive = l_file;
    l_info->a_bytesPerSector = l_bootRecord->a_bytesPerSector;
    l_info->a_sectorsPerCluster = l_bootRecord->a_sectorsPerCluster;
    l_info->a_bytesPerCluster =
        l_info->a_bytesPerSector * l_info->a_sectorsPerCluster;
    l_info->a_fatFirstSector = l_bootRecord->a_reservedSectors;
    l_info->a_fatFirstByte =
        l_info->a_fatFirstSector * l_info->a_bytesPerSector;
    l_info->a_sectorsPerFat = l_bootRecord->a_sectorsPerFat;
    l_info->a_bytesPerFat = l_info->a_sectorsPerFat * l_info->a_bytesPerSector;
    l_info->a_fatCount = l_bootRecord->a_fatCount;
    l_info->a_rootFirstSector =
        l_info->a_fatFirstSector + l_info->a_fatCount * l_info->a_sectorsPerFat;
    l_info->a_rootFirstByte =
        l_info->a_rootFirstSector * l_info->a_bytesPerSector;
    l_info->a_rootSectorCount = (
        (l_bootRecord->a_rootDirectoryEntryCount * 32)
        + (l_bootRecord->a_bytesPerSector - 1)
    ) / l_bootRecord->a_bytesPerSector;
    l_info->a_rootByteCount =
        l_info->a_rootSectorCount * l_info->a_bytesPerSector;
    l_info->a_rootEntryCount = l_bootRecord->a_rootDirectoryEntryCount;
    l_info->a_rootByteCount =
        l_info->a_rootSectorCount * l_info->a_bytesPerSector;
    l_info->a_clusterFirstSector =
        l_bootRecord->a_reservedSectors
        + (l_info->a_fatCount * l_info->a_sectorsPerFat)
        + l_info->a_rootSectorCount;
    l_info->a_clusterFirstByte =
        l_info->a_clusterFirstSector * l_info->a_bytesPerSector;

    uint32_t l_totalSectors;

    if(l_bootRecord->a_totalSectors == 0) {
        l_totalSectors = l_bootRecord->a_largeSectorCount;
    } else {
        l_totalSectors = l_bootRecord->a_totalSectors;
    }

    l_info->a_clusterCount =
        (l_totalSectors - l_info->a_clusterFirstSector)
        / l_info->a_sectorsPerCluster;

    // Allocate memory for the root info structure
    struct ts_fatfsFileInfo *l_rootInfo =
        kmalloc(sizeof(struct ts_fatfsFileInfo));

    if(l_rootInfo == NULL) {
        debug("fatfs: Failed to allocate memory for fatfs file structure.\n");
        vfsOperationClose(l_mountPoint);
        vfsOperationClose(l_file);
        kfree(l_info);
        return -ENOMEM;
    }

    l_rootInfo->a_info = l_info;
    l_rootInfo->a_size = l_info->a_rootByteCount;

    /* 0 is an invalid cluster number (it is normally used for free clusters),
    but here we use it to mean that the current file is the root directory. */
    l_rootInfo->a_cluster = 0;

    // Detect FAT type
    if(l_info->a_clusterCount < 4085) {
        debug("fatfs: Unsupported FAT12 filesystem.\n");
        vfsOperationClose(l_mountPoint);
        vfsOperationClose(l_file);
        kfree(l_info);
        kfree(l_rootInfo);
        return 1;
    } else if(l_info->a_clusterCount < 65525) {
        debug("fatfs: Detected FAT16 filesystem.\n");
        l_info->a_type = E_FATFSTYPE_FAT16;
        l_mountPoint->a_fsData = l_rootInfo;
        l_mountPoint->a_operations = &s_fatfsOperationsRoot;
    } else {
        debug("fatfs: Unsupported FAT32 filesystem.\n");
        vfsOperationClose(l_mountPoint);
        vfsOperationClose(l_file);
        kfree(l_info);
        kfree(l_rootInfo);
        return 1;
    }

    return 0;
}

static int64_t fatfsReadFatEntry(
    const struct ts_fatfsInfo *l_info,
    size_t l_entryIndex
) {
    if(l_info->a_type == E_FATFSTYPE_FAT16) {
        return fatfsReadFatEntry16(l_info, l_entryIndex);
    } else {
        return -EINVAL;
    }
}

static int64_t fatfsReadFatEntry16(
    const struct ts_fatfsInfo *l_info,
    size_t l_entryIndex
) {
    uint16_t l_buffer;

    int l_returnValue = vfsOperationRead(
        l_info->a_drive,
        l_info->a_fatFirstByte + (l_entryIndex << 1),
        &l_buffer,
        2
    );

    if(l_returnValue != 2) {
        return -EIO;
    }

    return l_buffer;
}

static int fatfsReadCluster(
    const struct ts_fatfsInfo *l_info,
    size_t l_clusterIndex,
    void *p_buffer,
    size_t p_size
) {
    return vfsOperationRead(
        l_info->a_drive,
        l_info->a_clusterFirstByte
            + l_clusterIndex * l_info->a_bytesPerCluster,
        p_buffer,
        p_size
    );
}
