#include <errno.h>
#include <string.h>

#include <kernel/arch/x86_64/inline.h>
#include <kernel/common.h>
#include <kernel/debug.h>
#include <kernel/module.h>
#include <kernel/fs/vfs.h>
#include <kernel/klibc/stdlib.h>
#include <modules/ata.h>
#include <modules/pci.h>

static int pciideInit(const char *p_args);
static void pciideQuit(void);
static void pciideScan(uint8_t p_bus, uint8_t p_slot, uint8_t p_function);

M_DECLARE_MODULE struct ts_module g_modulePciide = {
    .a_name = "pciide",
    .a_init = pciideInit,
    .a_quit = pciideQuit
};

static struct ts_vfsNode *s_pciDriver;

static int pciideInit(const char *p_args) {
    M_UNUSED_PARAMETER(p_args);

    debug("pciide: Scanning PCI bus...\n");

    int l_returnValue = vfsLookup(NULL, "/dev/pci", &s_pciDriver);

    if(l_returnValue != 0) {
        debug("pciide: Failed to find /dev/pci: %d\n", l_returnValue);
        return 1;
    }

    struct ts_pciRequestScan l_request = {
        .a_callback = pciideScan
    };

    vfsOperationIoctl(
        s_pciDriver,
        E_IOCTL_PCI_SCAN,
        &l_request
    );

    debug("pciide: Done.\n");

    vfsOperationClose(s_pciDriver);

    return 0;
}

static void pciideQuit(void) {

}

static void pciideScan(uint8_t p_bus, uint8_t p_slot, uint8_t p_function) {
    struct ts_pciRequestIdentification l_request = {
        .a_bus = p_bus,
        .a_slot = p_slot,
        .a_function = p_function
    };

    // Read device identification
    vfsOperationIoctl(
        s_pciDriver,
        E_IOCTL_PCI_IDENTIFY,
        &l_request
    );

    // Check if the device is supported
    if(l_request.a_class != 0x01 || l_request.a_subclass != 0x01) {
        return;
    }

    debug(
        "pciide: Found PCI IDE Controller at %02x:%02x.%x.\n",
        p_bus,
        p_slot,
        p_function
    );

    // Make sure that the ata module is loaded
    const struct ts_module *l_ataModule = moduleGetKernelModule("ata");

    if(l_ataModule == NULL) {
        debug("pciide: ata module was not found.\n");
        return;
    }

    if(!moduleIsModuleLoaded(l_ataModule)) {
        if(moduleLoad(l_ataModule, NULL) != 0) {
            debug("pciide: Failed to load ata module.\n");
            return;
        }
    }

    // Open the ata module driver file
    struct ts_vfsNode *l_ataDriver;
    int l_returnValue = vfsLookup(NULL, "/dev/ata", &l_ataDriver);

    if(l_returnValue != 0) {
        debug("pciide: Failed to find /dev/ata: %d.\n", l_returnValue);
        return;
    }

    // Register the primary IDE channel
    if((l_request.a_programmingInterface & 0x01) == 0x00) {
        struct ts_ataRequestCreate l_request2 = {
            .a_ioPortBase = 0x1f0,
            .a_ioPortControl = 0x3f6,
            .a_ioPortBusMaster = 0,
            .a_irq = 14
        };

        vfsOperationIoctl(
            l_ataDriver,
            E_IOCTL_ATA_DRIVER_CREATE,
            &l_request2
        );
    } else {
        debug("pciide: Primary channel is in PCI native mode (unsupported).\n");
    }

    // Register the secondary IDE channel
    if((l_request.a_programmingInterface & 0x04) == 0x00) {
        struct ts_ataRequestCreate l_request2 = {
            .a_ioPortBase = 0x170,
            .a_ioPortControl = 0x376,
            .a_ioPortBusMaster = 0,
            .a_irq = 15
        };

        vfsOperationIoctl(
            l_ataDriver,
            E_IOCTL_ATA_DRIVER_CREATE,
            &l_request2
        );
    } else {
        debug("pciide: Secondary channel is in PCI native mode (unsupported).\n");
    }

    vfsOperationClose(l_ataDriver);
}
