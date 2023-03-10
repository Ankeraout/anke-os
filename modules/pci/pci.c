#include <errno.h>
#include <string.h>

#include <kernel/arch/x86_64/inline.h>
#include <kernel/common.h>
#include <kernel/debug.h>
#include <kernel/module.h>
#include <kernel/fs/devfs.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <modules/pci.h>

#define C_IOPORT_PCI_CONFIG_ADDRESS 0xcf8
#define C_IOPORT_PCI_CONFIG_DATA 0xcfc

static int pciInit(const char *p_arg);
static void pciQuit(void);
static int pciIoctlDriver(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
);
static inline void pciConfigSetAddress(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint8_t p_offset
);
static uint8_t pciConfigRead8(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint8_t p_offset
);
static uint16_t pciConfigRead16(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint8_t p_offset
);
static uint32_t pciConfigRead32(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint8_t p_offset
);
static void pciConfigWrite8(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint8_t p_offset,
    uint8_t p_value
);
static void pciConfigWrite16(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint8_t p_offset,
    uint16_t p_value
);
static void pciConfigWrite32(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint8_t p_offset,
    uint32_t p_value
);
static void pciScan(tf_pciScanCallback *p_callback);
static void pciScanBus(tf_pciScanCallback *p_callback, uint8_t p_bus);
static void pciScanDevice(
    tf_pciScanCallback *p_callback,
    uint8_t p_bus,
    uint8_t p_slot
);
static void pciScanFunction(
    tf_pciScanCallback *p_callback,
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function
);
static void pciInitDevice(uint8_t p_bus, uint8_t p_slot, uint8_t p_function);
static void pciIdentify(struct ts_pciRequestIdentification *p_request);

M_DECLARE_MODULE struct ts_module g_modulePci = {
    .a_name = "pci",
    .a_init = pciInit,
    .a_quit = pciQuit
};

static int pciInit(const char *p_arg) {
    M_UNUSED_PARAMETER(p_arg);

    debug("pci: Initializing module...\n");

    // Create PCI driver file
    struct ts_vfsFileDescriptor *l_pciDriver =
        kcalloc(sizeof(struct ts_vfsFileDescriptor));

    if(l_pciDriver == NULL) {
        debug("pci: Failed to allocate memory for driver file.\n");
        return -ENOMEM;
    }

    strcpy(l_pciDriver->a_name, "pci");
    l_pciDriver->a_ioctl = pciIoctlDriver;
    l_pciDriver->a_type = E_VFS_FILETYPE_CHARACTER;

    // Register driver file
    struct ts_vfsFileDescriptor *l_dev = vfsOpen("/dev", 0);

    if(l_dev == NULL) {
        debug("pci: Failed to open /dev.\n");
        kfree(l_pciDriver);
        return -ENOENT;
    }

    int l_returnValue = l_dev->a_ioctl(
        l_dev,
        C_IOCTL_DEVFS_CREATE,
        l_pciDriver
    );

    kfree(l_dev);

    if(l_returnValue != 0) {
        debug("pci: Failed to create driver file.\n");
        kfree(l_pciDriver);
        return l_returnValue;
    }

    debug("pci: Registered /dev/pci.\n");
    debug("pci: Enumerating devices...\n");

    pciScan(pciInitDevice);

    debug("pci: Module initialized successfully.\n");

    return 0;
}

static void pciQuit(void) {

}

static int pciIoctlDriver(
    struct ts_vfsFileDescriptor *p_file,
    int p_request,
    void *p_arg
) {
    M_UNUSED_PARAMETER(p_file);

    switch(p_request) {
        case E_IOCTL_PCI_SCAN:
            pciScan(((struct ts_pciRequestScan *)p_arg)->a_callback);
            return 0;

        case E_IOCTL_PCI_IDENTIFY:
            pciIdentify(p_arg);
            return 0;

        default:
            return -EINVAL;
    }

    return -EINVAL;
}

static inline void pciConfigSetAddress(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint8_t p_offset
) {
    const uint32_t l_address =
        0x80000000
        | (p_bus << 16)
        | (p_slot << 11)
        | (p_function << 8)
        | (p_offset & 0xfc);

    outl(C_IOPORT_PCI_CONFIG_ADDRESS, l_address);
}

static uint8_t pciConfigRead8(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint8_t p_offset
) {
    const int l_shift = (3 - (p_offset & 0x03)) << 3;

    pciConfigSetAddress(p_bus, p_slot, p_function, p_offset);

    return (inl(C_IOPORT_PCI_CONFIG_DATA) >> l_shift) & 0xff;
}

static uint16_t pciConfigRead16(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint8_t p_offset
) {
    const int l_shift = (2 - (p_offset & 0x02)) << 3;

    pciConfigSetAddress(p_bus, p_slot, p_function, p_offset);

    return (inl(C_IOPORT_PCI_CONFIG_DATA) >> l_shift) & 0xffff;
}

static uint32_t pciConfigRead32(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint8_t p_offset
) {
    pciConfigSetAddress(p_bus, p_slot, p_function, p_offset);

    return inl(C_IOPORT_PCI_CONFIG_DATA);
}

static void pciConfigWrite8(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint8_t p_offset,
    uint8_t p_value
) {
    const int l_shift = (p_offset & 0x03) << 3;
    const uint32_t l_mask = ~(0xff << l_shift);

    pciConfigSetAddress(p_bus, p_slot, p_function, p_offset);

    uint32_t l_tmp = inl(C_IOPORT_PCI_CONFIG_DATA);
    l_tmp &= l_mask;
    l_tmp |= p_value << l_shift;

    outl(C_IOPORT_PCI_CONFIG_DATA, l_tmp);
}

static void pciConfigWrite16(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint8_t p_offset,
    uint16_t p_value
) {
    const int l_shift = (p_offset & 0x02) << 3;
    const uint32_t l_mask = ~(0xffff << l_shift);

    pciConfigSetAddress(p_bus, p_slot, p_function, p_offset);

    uint32_t l_tmp = inl(C_IOPORT_PCI_CONFIG_DATA);
    l_tmp &= l_mask;
    l_tmp |= p_value << l_shift;

    outl(C_IOPORT_PCI_CONFIG_DATA, l_tmp);
}

static void pciConfigWrite32(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint8_t p_offset,
    uint32_t p_value
) {
    pciConfigSetAddress(p_bus, p_slot, p_function, p_offset);
    outb(C_IOPORT_PCI_CONFIG_DATA, p_value);
}

static void pciScan(tf_pciScanCallback *p_callback) {
    for(int l_bus = 0; l_bus < 256; l_bus++) {
        pciScanBus(p_callback, l_bus);
    }
}

static void pciScanBus(tf_pciScanCallback *p_callback, uint8_t p_bus) {
    for(int l_slot = 0; l_slot < 32; l_slot++) {
        pciScanDevice(p_callback, p_bus, l_slot);
    }
}

static void pciScanDevice(
    tf_pciScanCallback *p_callback,
    uint8_t p_bus,
    uint8_t p_slot
) {
    uint16_t l_deviceId = pciConfigRead16(p_bus, p_slot, 0, 0);
    uint16_t l_vendorId = pciConfigRead16(p_bus, p_slot, 0, 2);

    if((l_deviceId == 0xffff) && (l_vendorId == 0xffff)) {
        // No device present
        return;
    }

    uint8_t l_headerType = pciConfigRead8(p_bus, p_slot, 0, 13);

    if((l_headerType & 0x80) != 0) {
        for(int l_func = 0; l_func < 8; l_func++) {
            pciScanFunction(p_callback, p_bus, p_slot, l_func);
        }
    } else {
        pciScanFunction(p_callback, p_bus, p_slot, 0);
    }
}

static void pciScanFunction(
    tf_pciScanCallback *p_callback,
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function
) {
    uint16_t l_deviceId = pciConfigRead16(p_bus, p_slot, p_function, 0);
    uint16_t l_vendorId = pciConfigRead16(p_bus, p_slot, p_function, 2);

    if((l_deviceId != 0xffff) || (l_vendorId != 0xffff)) {
        p_callback(p_bus, p_slot, p_function);
    }
}

static void pciInitDevice(uint8_t p_bus, uint8_t p_slot, uint8_t p_function) {
    uint16_t l_deviceId = pciConfigRead16(p_bus, p_slot, p_function, 0);
    uint16_t l_vendorId = pciConfigRead16(p_bus, p_slot, p_function, 2);
    uint8_t l_deviceClass = pciConfigRead8(p_bus, p_slot, p_function, 8);
    uint8_t l_deviceSubclass = pciConfigRead8(p_bus, p_slot, p_function, 9);
    uint8_t l_deviceProgrammingInterface = pciConfigRead8(p_bus, p_slot, p_function, 10);
    uint8_t l_deviceRevision = pciConfigRead8(p_bus, p_slot, p_function, 11);

    debug(
        "pci: %02x:%02x.%x: %04x:%04x (%02x:%02x:%02x:%02x)\n",
        p_bus,
        p_slot,
        p_function,
        l_vendorId,
        l_deviceId,
        l_deviceClass,
        l_deviceSubclass,
        l_deviceProgrammingInterface,
        l_deviceRevision
    );
}

static void pciIdentify(struct ts_pciRequestIdentification *p_request) {
    p_request->a_device = pciConfigRead16(p_request->a_bus, p_request->a_slot, p_request->a_function, 0);
    p_request->a_vendor = pciConfigRead16(p_request->a_bus, p_request->a_slot, p_request->a_function, 2);
    p_request->a_class = pciConfigRead8(p_request->a_bus, p_request->a_slot, p_request->a_function, 8);
    p_request->a_subclass = pciConfigRead8(p_request->a_bus, p_request->a_slot, p_request->a_function, 9);
    p_request->a_programmingInterface = pciConfigRead8(p_request->a_bus, p_request->a_slot, p_request->a_function, 10);
    p_request->a_revision = pciConfigRead8(p_request->a_bus, p_request->a_slot, p_request->a_function, 11);
}
