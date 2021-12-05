#include <stdbool.h>
#include <stdint.h>

#include "common.h"
#include "kernel/arch/x86/dev/pci.h"
#include "kernel/libk/stdio.h"
#include "kernel/libk/stdlib.h"

void vga_init();
static int initDevice(const pci_dev_t *dev);
static bool supportsDevice(uint16_t deviceId, uint16_t vendorId, uint8_t class, uint8_t subclass, uint8_t progIf);

void vga_init() {
    pci_driver_t *driver = malloc(sizeof(pci_driver_t));

    driver->deviceName = "VGA Compatible Controller";
    driver->initDevice = initDevice;
    driver->supportsDevice = supportsDevice;

    pci_registerDriver(driver);
}

static int initDevice(const pci_dev_t *dev) {
    UNUSED_PARAMETER(dev);

    printf("vga: initDevice() called\n");

    return 0;
}

static bool supportsDevice(uint16_t deviceId, uint16_t vendorId, uint8_t class, uint8_t subclass, uint8_t progIf) {
    UNUSED_PARAMETER(deviceId);
    UNUSED_PARAMETER(vendorId);
    UNUSED_PARAMETER(progIf);

    return (class == 0x03) && (subclass == 0x00);
}
