#include "dev/disk/disk.h"
#include "libk/list.h"
#include "libk/stdio.h"

static list_t *disk_deviceList;

void disk_init();
void disk_registerDevice(dev_disk_t *disk);

void disk_init() {
    disk_deviceList = list_create();
}

void disk_registerDevice(dev_disk_t *disk) {
    list_add(disk_deviceList, disk);

    char buffer[256];
    disk->api.getDeviceName(disk, buffer, 256);

    printf("disk: registered device: \"%s\"\n", buffer);
}
