#ifndef __INCLUDE_MODULES_PCI_H__
#define __INCLUDE_MODULES_PCI_H__

#include <stdint.h>

#include <kernel/module.h>

enum {
    E_MODULECALL_PCI_CONFIG_READ8 = C_MODULE_FIRST_FREE_CALL,
    E_MODULECALL_PCI_CONFIG_READ16,
    E_MODULECALL_PCI_CONFIG_READ32,
    E_MODULECALL_PCI_CONFIG_WRITE8,
    E_MODULECALL_PCI_CONFIG_WRITE16,
    E_MODULECALL_PCI_CONFIG_WRITE32
};

struct ts_pciDeviceAddress {
    uint8_t a_bus;
    uint8_t a_slot;
    uint8_t a_function;
    uint8_t a_offset;
};

struct ts_pciConfigOperation {
    struct ts_pciDeviceAddress a_address;

    union {
        uint8_t a_u8;
        uint16_t a_u16;
        uint32_t a_u32;
    } a_value;
};

#endif
