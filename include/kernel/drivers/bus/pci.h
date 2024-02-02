#ifndef __INCLUDE_DRIVERS_BUS_PCI_H__
#define __INCLUDE_DRIVERS_BUS_PCI_H__

#include <stdint.h>

/**
 * @brief Initializes the PCI bus.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 on success.
*/
int pciInit(void);

/**
 * @brief Scans the PCI configuration space and calls the given callback
 * function for each device found.
 * 
 * @param[in] p_callback The callback function to call for each detected PCI
 * device.
*/
void pciScan(
    void (*p_callback)(
        uint8_t p_bus,
        uint8_t p_slot,
        uint8_t p_function,
        uint16_t p_vendor,
        uint16_t p_device,
        uint8_t p_class,
        uint8_t p_subclass,
        uint8_t p_programmingInterface
    )
);

/**
 * @brief Reads a byte in the PCI configuration space at the given address.
 * 
 * @param[in] p_address The address to read the byte from.
 * 
 * @returns The read byte.
*/
uint8_t pciRead8(uint32_t p_address);

/**
 * @brief Reads a byte in the PCI configuration space at the given address.
 * 
 * @param[in] p_address The address to read the word from (16-bit aligned).
 * 
 * @returns The read word.
*/
uint16_t pciRead16(uint32_t p_address);

/**
 * @brief Reads a dword in the PCI configuration space at the given address.
 * 
 * @param[in] p_address The address to read the dword from (32-bit aligned).
 * 
 * @returns The read dword.
*/
uint32_t pciRead32(uint32_t p_address);

/**
 * @brief Writes a byte in the PCI configuration space at the given address.
 * 
 * @param[in] p_address The address to write the byte to.
 * @param[in] p_value The byte to write.
*/
void pciWrite8(uint32_t p_address, uint8_t p_value);

/**
 * @brief Writes a word in the PCI configuration space at the given address.
 * 
 * @param[in] p_address The address to write the word to (16-bit aligned).
 * @param[in] p_value The word to write.
*/
void pciWrite16(uint32_t p_address, uint16_t p_value);

/**
 * @brief Writes a dword in the PCI configuration space at the given address.
 * 
 * @param[in] p_address The address to write the dword to (32-bit aligned).
 * @param[in] p_value The dword to write.
*/
void pciWrite32(uint32_t p_address, uint32_t p_value);

/**
 * @brief Gets the address of the device header in the PCI configuration space.
 * 
 * @returns The address of the device header in the PCI configuration space.
*/
uint32_t pciGetAddress(
    uint8_t p_bus,
    uint8_t p_slot,
    uint8_t p_function
);

#endif
