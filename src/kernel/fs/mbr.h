#ifndef __KERNEL_FS_MBR_H__
#define __KERNEL_FS_MBR_H__

#include <stdbool.h>

#include "dev/disk.h"

/**
 * \brief Detects if the given disk contains a MBR partition table or not.
 * 
 * \param disk The disk to probe
 * 
 * \return 0 if the given disk contains a MBR partition table, 1 if the given
 * disk does not contain a partition table, and a negative value in case of
 * error.
 */ 
int fs_detectPartitionTable(dev_disk_t *disk);

#endif
