#ifndef __INCLUDE_ACPI_FADT_H__
#define __INCLUDE_ACPI_FADT_H__

#include "acpi/acpi.h"
#include "acpi/gas.h"
#include "acpi/sdt.h"

struct ts_acpi_fadt {
    struct ts_acpi_sdtHeader m_header;
    uint32_t m_firmwareCtrl;
    uint32_t m_dsdt;
    uint8_t m_reserved;
    uint8_t m_preferredPmProfile;
    uint16_t m_sciInterrupt;
    uint32_t m_smiCommandPort;
    uint8_t m_acpiEnable;
    uint8_t m_acpiDisable;
    uint8_t m_s4biosReq;
    uint8_t m_pStateControl;
    uint32_t m_pm1aEventBlock;
    uint32_t m_pm1bEventBlock;
    uint32_t m_pm1aControlBlock;
    uint32_t m_pm1bControlBlock;
    uint32_t m_pm2ControlBlock;
    uint32_t m_pmTimerBlock;
    uint32_t m_gpe0Block;
    uint32_t m_gpe1Block;
    uint8_t m_pm1EventLength;
    uint8_t m_pm1ControlLength;
    uint8_t m_pm2ControlLength;
    uint8_t m_pmTimerLength;
    uint8_t m_gpe0Length;
    uint8_t m_gpe1Length;
    uint8_t m_gpe1Base;
    uint8_t m_cStateControl;
    uint16_t m_worstC2Latency;
    uint16_t m_worstC3Latency;
    uint16_t m_flushSize;
    uint16_t m_flushStride;
    uint8_t m_dutyOffset;
    uint8_t m_dutyWidth;
    uint8_t m_dayAlarm;
    uint8_t m_monthAlarm;
    uint8_t m_century;
    uint16_t m_bootArchitectureFlags;
    uint8_t m_reserved2;
    uint32_t m_flags;
    struct ts_acpi_genericAddressStructure m_resetRegister;
    uint8_t m_resetValue;
    uint8_t m_reserved3[3];
    uint64_t m_xFirmwareControl;
    uint64_t m_xDsdt;
    struct ts_acpi_genericAddressStructure m_xPm1aEventBlock;
    struct ts_acpi_genericAddressStructure m_xPm1bEventBlock;
    struct ts_acpi_genericAddressStructure m_xPm1aControlBlock;
    struct ts_acpi_genericAddressStructure m_xPm1bControlBlock;
    struct ts_acpi_genericAddressStructure m_xPm2ControlBlock;
    struct ts_acpi_genericAddressStructure m_xPmTimerBlock;
    struct ts_acpi_genericAddressStructure m_xGpe0Block;
    struct ts_acpi_genericAddressStructure m_xGpe1Block;
} __attribute__((packed));

int acpi_fadtInit(struct ts_acpi *p_acpi);

#endif
