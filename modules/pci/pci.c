#include <string.h>

#include <kernel/arch/x86_64/inline.h>
#include <kernel/common.h>
#include <kernel/debug.h>
#include <kernel/module.h>
#include <modules/pci.h>

#define C_IOPORT_PCI_CONFIG_ADDRESS 0xcf8
#define C_IOPORT_PCI_CONFIG_DATA 0xcfc

static int pciInit(void);
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
static void pciCheckDevice(uint8_t p_bus, uint8_t p_slot);
static void pciCheckDeviceFunction(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function
);
static void pciInitDevice(uint8_t p_bus, uint8_t p_slot, uint8_t p_function);
static int pciCall(int l_operation, void *p_arg);

static int pciInit(void) {
    for(int l_bus = 0; l_bus < 256; l_bus++) {
        for(int l_slot = 0; l_slot < 32; l_slot++) {
            pciCheckDevice(l_bus, l_slot);
        }
    }

    return E_MODULECALLRESULT_SUCCESS;
}

static void pciQuit(void) {

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

static void pciCheckDevice(uint8_t p_bus, uint8_t p_slot) {
    uint16_t l_deviceId = pciConfigRead16(p_bus, p_slot, 0, 0);
    uint16_t l_vendorId = pciConfigRead16(p_bus, p_slot, 0, 2);

    if((l_deviceId == 0xffff) && (l_vendorId == 0xffff)) {
        // No device present
        return;
    }

    uint8_t l_headerType = pciConfigRead8(p_bus, p_slot, 0, 13);

    if((l_headerType & 0x80) != 0) {
        for(int l_func = 0; l_func < 8; l_func++) {
            pciCheckDeviceFunction(p_bus, p_slot, l_func);
        }
    } else {
        pciCheckDeviceFunction(p_bus, p_slot, 0);
    }
}

static void pciCheckDeviceFunction(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function
) {
    uint16_t l_deviceId = pciConfigRead16(p_bus, p_slot, p_function, 0);
    uint16_t l_vendorId = pciConfigRead16(p_bus, p_slot, p_function, 2);

    if((l_deviceId != 0xffff) || (l_vendorId != 0xffff)) {
        pciInitDevice(p_bus, p_slot, p_function);
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

static int pciCall(int l_operation, void *p_arg) {
    switch(l_operation) {
        case E_MODULECALL_INIT: return pciInit();

        case E_MODULECALL_QUIT:
            pciQuit();
            return E_MODULECALLRESULT_SUCCESS;

        case E_MODULECALL_PCI_CONFIG_READ8:
            {
                struct ts_pciConfigOperation *l_operation =
                    (struct ts_pciConfigOperation *)p_arg;

                l_operation->a_value.a_u8 = pciConfigRead8(
                    l_operation->a_address.a_bus,
                    l_operation->a_address.a_slot,
                    l_operation->a_address.a_function,
                    l_operation->a_address.a_offset
                );
            }

            return E_MODULECALLRESULT_SUCCESS;

        case E_MODULECALL_PCI_CONFIG_READ16:
            {
                struct ts_pciConfigOperation *l_operation =
                    (struct ts_pciConfigOperation *)p_arg;

                l_operation->a_value.a_u16 = pciConfigRead8(
                    l_operation->a_address.a_bus,
                    l_operation->a_address.a_slot,
                    l_operation->a_address.a_function,
                    l_operation->a_address.a_offset
                );
            }

            return E_MODULECALLRESULT_SUCCESS;

        case E_MODULECALL_PCI_CONFIG_READ32:
            {
                struct ts_pciConfigOperation *l_operation =
                    (struct ts_pciConfigOperation *)p_arg;

                l_operation->a_value.a_u32 = pciConfigRead32(
                    l_operation->a_address.a_bus,
                    l_operation->a_address.a_slot,
                    l_operation->a_address.a_function,
                    l_operation->a_address.a_offset
                );
            }

            return E_MODULECALLRESULT_SUCCESS;

        case E_MODULECALL_PCI_CONFIG_WRITE8:
            {
                struct ts_pciConfigOperation *l_operation =
                    (struct ts_pciConfigOperation *)p_arg;

                pciConfigWrite8(
                    l_operation->a_address.a_bus,
                    l_operation->a_address.a_slot,
                    l_operation->a_address.a_function,
                    l_operation->a_address.a_offset,
                    l_operation->a_value.a_u8
                );
            }

            return E_MODULECALLRESULT_SUCCESS;

        case E_MODULECALL_PCI_CONFIG_WRITE16:
            {
                struct ts_pciConfigOperation *l_operation =
                    (struct ts_pciConfigOperation *)p_arg;

                pciConfigWrite16(
                    l_operation->a_address.a_bus,
                    l_operation->a_address.a_slot,
                    l_operation->a_address.a_function,
                    l_operation->a_address.a_offset,
                    l_operation->a_value.a_u16
                );
            }

            return E_MODULECALLRESULT_SUCCESS;

        case E_MODULECALL_PCI_CONFIG_WRITE32:
            {
                struct ts_pciConfigOperation *l_operation =
                    (struct ts_pciConfigOperation *)p_arg;

                pciConfigWrite32(
                    l_operation->a_address.a_bus,
                    l_operation->a_address.a_slot,
                    l_operation->a_address.a_function,
                    l_operation->a_address.a_offset,
                    l_operation->a_value.a_u32
                );
            }

            return E_MODULECALLRESULT_SUCCESS;

        default:
            return E_MODULECALLRESULT_NOT_IMPLEMENTED;
    }
}

M_DECLARE_MODULE struct ts_module g_modulePci = {
    .a_name = "pci",
    .a_call = pciCall
};
