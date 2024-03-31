#ifndef __INCLUDE_KERNEL_DRIVERS_BLOCK_PATA_PATA_H__
#define __INCLUDE_KERNEL_DRIVERS_BLOCK_PATA_PATA_H__

#include <stdint.h>

enum te_pataErrorCode {
    E_PATAERRORCODE_NONE,
    E_PATAERRORCODE_NODRIVE,
    E_PATAERRORCODE_ATAPI,
    E_PATAERRORCODE_SATA,
    E_PATAERRORCODE_SATAPI
};

enum te_pataIoRegister {
    E_PATA_IO_REGISTER_DATA = 0,
    E_PATA_IO_REGISTER_ERROR = 1,
    E_PATA_IO_REGISTER_FEATURES = 1,
    E_PATA_IO_REGISTER_SECTOR_COUNT = 2,
    E_PATA_IO_REGISTER_CHS_SECTOR = 3,
    E_PATA_IO_REGISTER_LBA_LOW = 3,
    E_PATA_IO_REGISTER_CHS_CYLINDER_LOW = 4,
    E_PATA_IO_REGISTER_LBA_MID = 4,
    E_PATA_IO_REGISTER_CHS_CYLINDER_HIGH = 5,
    E_PATA_IO_REGISTER_LBA_HIGH = 5,
    E_PATA_IO_REGISTER_DRIVE_HEAD = 6,
    E_PATA_IO_REGISTER_STATUS = 7,
    E_PATA_IO_REGISTER_COMMAND = 7
};

enum te_pataControlRegister {
    E_PATA_CTL_REGISTER_STATUS = 0,
    E_PATA_CTL_REGISTER_CONTROL = 0,
    E_PATA_CTL_REGISTER_DRIVE_ADDRESS = 1
};

struct ts_pataChannelOperations {
    void *m_driverData;
    uint8_t (*m_readIo8)(
        struct ts_pataChannelOperations *p_operations,
        enum te_pataIoRegister p_register
    );
    uint8_t (*m_readCtl8)(
        struct ts_pataChannelOperations *p_operations,
        enum te_pataControlRegister p_register
    );
    uint16_t (*m_readIo16)(
        struct ts_pataChannelOperations *p_operations,
        enum te_pataIoRegister p_register
    );
    void (*m_writeIo8)(
        struct ts_pataChannelOperations *p_operations,
        enum te_pataIoRegister p_register, uint8_t p_value
    );
    void (*m_writeCtl8)(
        struct ts_pataChannelOperations *p_operations,
        enum te_pataControlRegister p_register, uint8_t p_value
    );
    void (*m_writeIo16)(
        struct ts_pataChannelOperations *p_operations,
        enum te_pataIoRegister p_register, uint16_t p_value
    );
};

/**
 * @brief Initializes the PATA subsystem.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success.
*/
int pataInit(void);

/**
 * @brief Registers a PATA controller.
 * 
 * @returns The controller ID if the operation was successful, or a negative
 * error number on error.
*/
int pataRegisterController(struct ts_pataChannelOperations *p_operations);

#endif
