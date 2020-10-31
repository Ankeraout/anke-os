#ifndef __FADT_H__
#define __FADT_H__

#include <stdint.h>

#include "acpi/acpi.h"

typedef struct {
    uint8_t addressSpace;
    uint8_t bitWidth;
    uint8_t bitOffset;
    uint8_t accessSize;
    uint64_t address;
} __attribute__((packed)) fadt_gas_t;

typedef struct {
    acpi_sdt_header_t header;
    uint32_t facsPtr;
    uint32_t dsdtPtr;
    uint8_t interruptModel;
    uint8_t powerManagementProfile;
    uint16_t sciInterruptNumber;
    uint32_t smiCommandPort;
    uint8_t acpiEnable;
    uint8_t acpiDisable;
    uint8_t s4biosReq;
    uint8_t pstateControl;
    uint32_t pm1a_eventRegisterBlockPort;
    uint32_t pm1b_eventRegisterBlockPort;
    uint32_t pm1a_controlRegisterBlockPort;
    uint32_t pm1b_controlRegisterBlockPort;
    uint32_t pm2_controlRegisterBlockPort;
    uint32_t pm_timerControlRegisterBlockPort;
    uint32_t gpe0_registerBlockPort;
    uint32_t gpe1_registerBlockPort;
    uint8_t pm1_eventRegisterBlockLength;
    uint8_t pm1_controlRegisterBlockLength;
    uint8_t pm2_controlRegisterBlockLength;
    uint8_t pm_timerControlRegisterBlockLength;
    uint8_t gpe0_registerBlockLength;
    uint8_t gpe1_registerBlockLength;
    uint8_t gpe1_base;
    uint8_t cstSupported;
    uint16_t worstLatencyC2;
    uint16_t worstLatencyC3;
    uint16_t flushSize;
    uint16_t flushStride;
    uint8_t dutyOffset;
    uint8_t dutyWidth;
    uint8_t dayAlarm;
    uint8_t monthAlarm;
    uint8_t century;
    uint16_t iapcBootArchitectureFlags;
    uint8_t reserved;
    uint32_t flags;
    fadt_gas_t resetRegister;
    uint8_t resetValue;
    uint16_t armBootArchitectureFlags;
    uint8_t fadtMinorVersion;
    uint64_t xFacs;
    uint64_t xDsdt;
    fadt_gas_t xPm1a_eventRegisterBlock;
    fadt_gas_t xPm1b_eventRegisterBlock;
    fadt_gas_t xPm1a_controlRegisterBlock;
    fadt_gas_t xPm1b_controlRegisterBlock;
    fadt_gas_t xPm2_controlRegisterBlock;
    fadt_gas_t xPm_timerControlRegisterBlock;
    fadt_gas_t xGpe0_registerBlock;
    fadt_gas_t xGpe1_registerBlock;
    fadt_gas_t sleepControlRegister;
    fadt_gas_t sleepStatusRegister;
} __attribute__((packed)) fadt_t;

#endif
