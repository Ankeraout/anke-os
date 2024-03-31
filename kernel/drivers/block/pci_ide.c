#include "kernel/drivers/block/pata/pata.h"
#include "kernel/drivers/block/pci_ide.h"
#include "kernel/drivers/bus/pci.h"
#include "klibc/debug.h"
#include "klibc/stdlib.h"

static void pciideInitCallback(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint16_t p_vendor,
    uint16_t p_device,
    uint8_t p_class,
    uint8_t p_subclass,
    uint8_t p_programmingInterface
);

int pciideInit(void) {
    pciScan(pciideInitCallback);
    return 0;
}

static void pciideInitCallback(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function,
    uint16_t p_vendor,
    uint16_t p_device,
    uint8_t p_class,
    uint8_t p_subclass,
    uint8_t p_programmingInterface
) {
    if(p_class != 0x01 || p_subclass != 0x01) {
        return;
    }

    kernelDebug(
        "pci_ide: Found PCI IDE controller at %02x:%02x.%x:\n",
        p_bus,
        p_slot,
        p_function
    );
    
    kernelDebug("pci_ide: - Bus mastering: ");

    if((p_programmingInterface & (1 << 7)) != 0) {
        kernelDebug("yes\n");

        uint32_t l_pciAddress = pciGetAddress(p_bus, p_slot, p_function);
        uint32_t l_bar4 = pciRead32(l_pciAddress + 0x20);

        kernelDebug(
            "pci_ide: - Bus mastering I/O base: 0x%x",
            l_bar4 & 0xfffffffc
        );
    } else {
        kernelDebug("no");
    }
    
    kernelDebug("\npci_ide: - Channel 0:\n");

    if((p_programmingInterface & (1 << 0)) != 0) {
        kernelDebug("pci_ide:   - Mode: PCI\n");

        uint32_t l_pciAddress = pciGetAddress(p_bus, p_slot, p_function);
        uint32_t l_bar0 = pciRead32(l_pciAddress + 0x10);
        uint32_t l_bar1 = pciRead32(l_pciAddress + 0x14);

        kernelDebug("pci_ide:   - I/O base: 0x%03x\n", l_bar0 & 0xfffffffc);
        kernelDebug("pci_ide:   - I/O control: 0x%03x\n", l_bar1 & 0xfffffffc);
        kernelDebug("pci_ide:   - IRQ: %d\n", pciRead8(l_pciAddress + 0x3f));
    } else {
        kernelDebug("pci_ide:   - Mode: Compatibility\n");
        kernelDebug("pci_ide:   - I/O base: 0x1f0\n");
        kernelDebug("pci_ide:   - I/O control: 0x3f6\n");
        kernelDebug("pci_ide:   - IRQ: 14\n");
    }

    if((p_programmingInterface & (1 << 1)) != 0) {
        kernelDebug("pci_ide:   - Mode switch support: yes\n");
    } else {
        kernelDebug("pci_ide:   - Mode switch support: no\n");
    }
    
    kernelDebug("pci_ide: - Channel 1:\n");

    if((p_programmingInterface & (1 << 2)) != 0) {
        kernelDebug("pci_ide:   - Mode: PCI\n");

        uint32_t l_pciAddress = pciGetAddress(p_bus, p_slot, p_function);
        uint32_t l_bar2 = pciRead32(l_pciAddress + 0x18);
        uint32_t l_bar3 = pciRead32(l_pciAddress + 0x1c);

        kernelDebug("pci_ide    - I/O base: 0x%03x\n", l_bar2 & 0xfffffffc);
        kernelDebug("pci_ide    - I/O control: 0x%03x\n", l_bar3 & 0xfffffffc);
        kernelDebug("pci_ide:   - IRQ: %d\n", pciRead8(l_pciAddress + 0x3f));
    } else {
        kernelDebug("pci_ide:   - Mode: Compatibility\n");
        kernelDebug("pci_ide:   - I/O base: 0x170\n");
        kernelDebug("pci_ide:   - I/O control: 0x376\n");
        kernelDebug("pci_ide:   - IRQ: 15\n");
    }

    if((p_programmingInterface & (1 << 3)) != 0) {
        kernelDebug("pci_ide:   - Mode switch support: yes\n");
    } else {
        kernelDebug("pci_ide:   - Mode switch support: no\n");
    }
}
